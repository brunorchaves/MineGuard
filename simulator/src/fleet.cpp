#include "fleet.hpp"
#include <cmath>
#include <iostream>

namespace mineguard {

// ============================================================
// Layout da mina - waypoints baseados em coordenadas reais
// Inspirado em uma mina a ceu aberto em Minas Gerais
// ============================================================

static constexpr double DEG_TO_RAD = M_PI / 180.0;
static constexpr double EARTH_RADIUS = 6371000.0;

MineLayout MineLayout::create_default() {
    MineLayout layout;

    // Area de escavacao (fundo do pit)
    layout.waypoints["PIT_LOAD_1"]  = {-20.12200, -43.95200, 820.0};
    layout.waypoints["PIT_LOAD_2"]  = {-20.12250, -43.95150, 820.0};

    // Rampa de saida do pit
    layout.waypoints["RAMP_BOT"]    = {-20.12100, -43.95100, 840.0};
    layout.waypoints["RAMP_MID"]    = {-20.12000, -43.95000, 860.0};
    layout.waypoints["RAMP_TOP"]    = {-20.11900, -43.94900, 880.0};

    // Estrada principal
    layout.waypoints["ROAD_1"]      = {-20.11800, -43.94800, 890.0};
    layout.waypoints["ROAD_2"]      = {-20.11700, -43.94700, 895.0};

    // Area de descarga
    layout.waypoints["DUMP_APPROACH"] = {-20.11600, -43.94600, 900.0};
    layout.waypoints["DUMP_1"]      = {-20.11550, -43.94550, 900.0};
    layout.waypoints["DUMP_2"]      = {-20.11500, -43.94600, 900.0};

    // Rota do veiculo leve (patrulha de seguranca)
    layout.waypoints["PATROL_1"]    = {-20.11950, -43.94950, 870.0};
    layout.waypoints["PATROL_2"]    = {-20.11750, -43.94750, 892.0};
    layout.waypoints["PATROL_3"]    = {-20.11600, -43.94650, 898.0};
    layout.waypoints["PATROL_4"]    = {-20.11850, -43.94850, 885.0};

    return layout;
}

// ============================================================
// FleetManager
// ============================================================

FleetManager::FleetManager() {}

void FleetManager::initialize() {
    mine_ = MineLayout::create_default();
    create_fleet();
    setup_routes();
}

// --- Criacao da frota ---

void FleetManager::create_fleet() {
    // 3 Haul Trucks - comecam em posicoes diferentes do ciclo
    vehicles_.push_back(std::make_unique<Vehicle>(
        "HT-101", VehicleType::HAUL_TRUCK, mine_.waypoints["PIT_LOAD_1"]
    ));
    vehicles_.push_back(std::make_unique<Vehicle>(
        "HT-102", VehicleType::HAUL_TRUCK, mine_.waypoints["ROAD_1"]
    ));
    vehicles_.push_back(std::make_unique<Vehicle>(
        "HT-103", VehicleType::HAUL_TRUCK, mine_.waypoints["DUMP_APPROACH"]
    ));

    // 1 Excavator - fica fixo na area de carga
    vehicles_.push_back(std::make_unique<Vehicle>(
        "EX-201", VehicleType::EXCAVATOR, mine_.waypoints["PIT_LOAD_1"]
    ));

    // 1 Light Vehicle - patrulha de seguranca
    vehicles_.push_back(std::make_unique<Vehicle>(
        "LV-301", VehicleType::LIGHT_VEHICLE, mine_.waypoints["PATROL_1"]
    ));
}

// --- Setup inicial de rotas e estado de navegacao ---

void FleetManager::setup_routes() {
    // HT-101: comeca carregando no pit
    nav_states_["HT-101"] = NavigationState{
        .current_route = get_haul_route(),
        .current_waypoint_index = 0,
        .arrival_threshold = 20.0,
        .wait_timer = LOADING_TIME,
        .waiting = true
    };
    vehicles_[0]->set_cycle_state(CycleState::LOADING);
    vehicles_[0]->set_target_speed(0);

    // HT-102: ja na estrada, hauling com carga
    nav_states_["HT-102"] = NavigationState{
        .current_route = get_haul_route(),
        .current_waypoint_index = 4, // ROAD_1 em diante
        .arrival_threshold = 20.0,
        .wait_timer = 0,
        .waiting = false
    };
    vehicles_[1]->set_cycle_state(CycleState::HAULING);
    vehicles_[1]->set_target_speed(HAUL_SPEED);

    // HT-103: chegando no dump
    nav_states_["HT-103"] = NavigationState{
        .current_route = get_haul_route(),
        .current_waypoint_index = 6, // DUMP_APPROACH em diante
        .arrival_threshold = 20.0,
        .wait_timer = 0,
        .waiting = false
    };
    vehicles_[2]->set_cycle_state(CycleState::HAULING);
    vehicles_[2]->set_target_speed(APPROACH_SPEED);

    // EX-201: escavadeira fica parada operando
    nav_states_["EX-201"] = NavigationState{
        .current_route = Route{{"PIT_LOAD_1"}},
        .current_waypoint_index = 0,
        .arrival_threshold = 5.0,
        .wait_timer = 0,
        .waiting = true
    };
    vehicles_[3]->set_cycle_state(CycleState::IDLE);
    vehicles_[3]->set_target_speed(0);

    // LV-301: patrulha circulando
    nav_states_["LV-301"] = NavigationState{
        .current_route = Route{{"PATROL_1", "PATROL_2", "PATROL_3", "PATROL_4"}},
        .current_waypoint_index = 1,
        .arrival_threshold = 15.0,
        .wait_timer = 0,
        .waiting = false
    };
    vehicles_[4]->set_cycle_state(CycleState::HAULING); // "em transito"
    vehicles_[4]->set_target_speed(LV_PATROL_SPEED);

    // Setar headings iniciais pra quem esta em movimento
    for (auto& v : vehicles_) {
        auto& nav = nav_states_[v->id()];
        if (!nav.waiting && nav.current_waypoint_index < nav.current_route.waypoint_names.size()) {
            const auto& target_name = nav.current_route.waypoint_names[nav.current_waypoint_index];
            double heading = calculate_heading(v->position(), mine_.waypoints[target_name]);
            v->set_heading(heading);
        }
    }

    // Payload inicial pra quem ta carregado
    // HT-102 e HT-103 ja estao em rota com carga
    vehicles_[1]->set_target_speed(HAUL_SPEED); // reafirma
    vehicles_[2]->set_target_speed(HAUL_SPEED);
}

// --- Rotas ---

Route FleetManager::get_haul_route() const {
    // PIT -> DUMP (carregado)
    return Route{{
        "PIT_LOAD_1", "RAMP_BOT", "RAMP_MID", "RAMP_TOP",
        "ROAD_1", "ROAD_2", "DUMP_APPROACH", "DUMP_1"
    }};
}

Route FleetManager::get_return_route() const {
    // DUMP -> PIT (vazio)
    return Route{{
        "DUMP_1", "DUMP_APPROACH", "ROAD_2", "ROAD_1",
        "RAMP_TOP", "RAMP_MID", "RAMP_BOT", "PIT_LOAD_1"
    }};
}

// ============================================================
// Update principal - chamado a cada tick
// ============================================================

void FleetManager::update(double delta_time) {
    for (auto& vehicle : vehicles_) {
        auto& nav = nav_states_[vehicle->id()];

        // Escavadeira fica parada
        if (vehicle->type() == VehicleType::EXCAVATOR) {
            vehicle->update(delta_time);
            continue;
        }

        update_navigation(*vehicle, nav, delta_time);
        vehicle->update(delta_time);
    }
}

// --- Navegacao: mover veiculo entre waypoints ---

void FleetManager::update_navigation(Vehicle& vehicle, NavigationState& nav, double dt) {
    // Se ta esperando (loading/dumping), conta o timer
    if (nav.waiting) {
        nav.wait_timer -= dt;
        if (nav.wait_timer <= 0) {
            nav.waiting = false;
            advance_cycle(vehicle, nav);
        }
        return;
    }

    // Verifica se nao ha rota
    if (nav.current_route.waypoint_names.empty()) return;
    if (nav.current_waypoint_index >= nav.current_route.waypoint_names.size()) return;

    // Pega waypoint alvo
    const auto& target_name = nav.current_route.waypoint_names[nav.current_waypoint_index];
    const auto& target_pos = mine_.waypoints.at(target_name);

    // Calcula distancia ate o waypoint
    double dist = calculate_distance(vehicle.position(), target_pos);

    // Atualiza heading em direcao ao waypoint
    double heading = calculate_heading(vehicle.position(), target_pos);
    vehicle.set_heading(heading);

    // Chegou no waypoint?
    if (dist < nav.arrival_threshold) {
        nav.current_waypoint_index++;

        // Chegou no final da rota?
        if (nav.current_waypoint_index >= nav.current_route.waypoint_names.size()) {
            handle_route_complete(vehicle, nav);
            return;
        }

        // Reduz velocidade perto do proximo waypoint se for curva
        const auto& next_name = nav.current_route.waypoint_names[nav.current_waypoint_index];
        const auto& next_pos = mine_.waypoints.at(next_name);
        double new_heading = calculate_heading(vehicle.position(), next_pos);
        double heading_diff = std::abs(new_heading - vehicle.telemetry().heading);
        if (heading_diff > 180) heading_diff = 360 - heading_diff;

        if (heading_diff > 30) {
            vehicle.set_target_speed(APPROACH_SPEED);
        }
    }
}

// --- Transicao de ciclo quando rota termina ---

void FleetManager::advance_cycle(Vehicle& vehicle, NavigationState& nav) {
    CycleState current = vehicle.cycle_state();

    switch (current) {
        case CycleState::LOADING:
            // Terminou de carregar -> vai pro dump
            vehicle.set_cycle_state(CycleState::HAULING);
            nav.current_route = get_haul_route();
            nav.current_waypoint_index = 1; // pula PIT_LOAD, ja ta la
            vehicle.set_target_speed(HAUL_SPEED);
            break;

        case CycleState::DUMPING:
            // Terminou de descarregar -> volta pro pit
            vehicle.set_cycle_state(CycleState::RETURNING);
            nav.current_route = get_return_route();
            nav.current_waypoint_index = 1; // pula DUMP_1, ja ta la
            vehicle.set_target_speed(RETURN_SPEED);
            break;

        default:
            break;
    }
}

// --- Funcao privada: quando veiculo chega no fim da rota ---

void FleetManager::handle_route_complete(Vehicle& vehicle, NavigationState& nav) {
    vehicle.set_target_speed(0);

    if (vehicle.type() == VehicleType::LIGHT_VEHICLE) {
        // Veiculo leve: reinicia patrulha circular
        nav.current_route = Route{{"PATROL_1", "PATROL_2", "PATROL_3", "PATROL_4"}};
        nav.current_waypoint_index = 0;
        vehicle.set_target_speed(LV_PATROL_SPEED);
        return;
    }

    CycleState current = vehicle.cycle_state();

    if (current == CycleState::HAULING) {
        // Chegou no dump -> comeca a descarregar
        vehicle.set_cycle_state(CycleState::DUMPING);
        nav.waiting = true;
        nav.wait_timer = DUMPING_TIME;
    }
    else if (current == CycleState::RETURNING) {
        // Voltou pro pit -> comeca a carregar
        vehicle.set_cycle_state(CycleState::LOADING);
        nav.waiting = true;
        nav.wait_timer = LOADING_TIME;
    }
}

// --- Coleta de telemetria de todos os veiculos ---

std::vector<TelemetryPacket> FleetManager::collect_telemetry() const {
    std::vector<TelemetryPacket> packets;
    packets.reserve(vehicles_.size());
    for (const auto& v : vehicles_) {
        packets.push_back(v->generate_packet());
    }
    return packets;
}

// ============================================================
// Funcoes de geometria
// ============================================================

double FleetManager::calculate_heading(const Position& from, const Position& to) const {
    double dlat = to.latitude - from.latitude;
    double dlon = to.longitude - from.longitude;

    // Correcao pela latitude
    double cos_lat = std::cos(from.latitude * DEG_TO_RAD);
    double dx = dlon * cos_lat;
    double dy = dlat;

    double heading_rad = std::atan2(dx, dy);
    double heading_deg = heading_rad * (180.0 / M_PI);

    if (heading_deg < 0) heading_deg += 360.0;
    return heading_deg;
}

double FleetManager::calculate_distance(const Position& a, const Position& b) const {
    // Haversine simplificado pra distancias curtas
    double dlat = (b.latitude - a.latitude) * DEG_TO_RAD;
    double dlon = (b.longitude - a.longitude) * DEG_TO_RAD;
    double cos_lat = std::cos(a.latitude * DEG_TO_RAD);

    double dx = dlon * cos_lat * EARTH_RADIUS;
    double dy = dlat * EARTH_RADIUS;

    return std::sqrt(dx * dx + dy * dy);
}

} // namespace mineguard

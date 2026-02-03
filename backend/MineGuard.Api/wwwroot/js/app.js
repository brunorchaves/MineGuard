// ============================================================
// MineGuard Dashboard Application
// ============================================================

const API_BASE = 'http://localhost:5100/api';
const POLL_INTERVAL = 1000; // 1 segundo

// ============================================================
// Mapa Leaflet
// ============================================================

const map = L.map('map', {
    zoomControl: true,
    attributionControl: false
}).setView([-20.1190, -43.9490], 16);

// Tile layer - CartoDB Dark (combina com o tema)
L.tileLayer('https://{s}.basemaps.cartocdn.com/dark_all/{z}/{x}/{y}{r}.png', {
    maxZoom: 19
}).addTo(map);

// Marcadores dos veiculos (guardados por id)
const vehicleMarkers = {};
const vehicleTrails = {};

// Icones customizados por tipo de veiculo
function createVehicleIcon(vehicleType) {
    const colors = {
        0: '#3b82f6', // HaulTruck - azul
        1: '#f97316', // Excavator - laranja
        2: '#22c55e'  // LightVehicle - verde
    };

    const labels = {
        0: 'HT',
        1: 'EX',
        2: 'LV'
    };

    const color = colors[vehicleType] || '#8b91a8';
    const label = labels[vehicleType] || '??';

    return L.divIcon({
        className: 'vehicle-marker',
        html: `
            <div style="
                background: ${color};
                width: 32px;
                height: 32px;
                border-radius: 50%;
                display: flex;
                align-items: center;
                justify-content: center;
                font-weight: 700;
                font-size: 11px;
                color: white;
                border: 2px solid white;
                box-shadow: 0 0 12px ${color}80;
                font-family: 'Inter', sans-serif;
            ">${label}</div>
        `,
        iconSize: [32, 32],
        iconAnchor: [16, 16]
    });
}

// Circulo de raio de seguranca
const safetyCircles = {};

function getSafetyRadius(vehicleType) {
    switch (vehicleType) {
        case 0: return 30;  // HaulTruck
        case 1: return 25;  // Excavator
        case 2: return 10;  // LightVehicle
        default: return 15;
    }
}

// ============================================================
// Atualizar veiculos no mapa
// ============================================================

function updateMap(vehicles) {
    vehicles.forEach(v => {
        const lat = v.position.latitude;
        const lng = v.position.longitude;
        const pos = [lat, lng];

        if (vehicleMarkers[v.id]) {
            // Atualizar posicao existente
            vehicleMarkers[v.id].setLatLng(pos);
            vehicleMarkers[v.id].setIcon(createVehicleIcon(v.vehicleType));
            safetyCircles[v.id].setLatLng(pos);

            // Trail (rastro)
            if (vehicleTrails[v.id]) {
                const trail = vehicleTrails[v.id];
                const latlngs = trail.getLatLngs();
                latlngs.push(pos);
                // Manter apenas ultimos 60 pontos (1 min)
                if (latlngs.length > 60) latlngs.shift();
                trail.setLatLngs(latlngs);
            }
        } else {
            // Criar marcador novo
            const marker = L.marker(pos, {
                icon: createVehicleIcon(v.vehicleType)
            }).addTo(map);

            // Popup com info do veiculo
            marker.bindPopup('');
            vehicleMarkers[v.id] = marker;

            // Raio de seguranca
            const colors = { 0: '#3b82f6', 1: '#f97316', 2: '#22c55e' };
            safetyCircles[v.id] = L.circle(pos, {
                radius: getSafetyRadius(v.vehicleType),
                color: colors[v.vehicleType] || '#8b91a8',
                fillColor: colors[v.vehicleType] || '#8b91a8',
                fillOpacity: 0.08,
                weight: 1,
                opacity: 0.3
            }).addTo(map);

            // Trail
            vehicleTrails[v.id] = L.polyline([pos], {
                color: colors[v.vehicleType] || '#8b91a8',
                weight: 2,
                opacity: 0.4
            }).addTo(map);
        }

        // Atualizar popup
        vehicleMarkers[v.id].setPopupContent(`
            <div style="font-family: Inter, sans-serif; font-size: 13px; min-width: 180px;">
                <strong style="font-size: 15px;">${v.id}</strong>
                <span style="opacity:0.6; margin-left:6px;">${v.vehicleTypeName}</span>
                <hr style="border:none; border-top:1px solid #ddd; margin:6px 0;">
                <div><b>State:</b> ${v.cycleStateName}</div>
                <div><b>Speed:</b> ${v.telemetry.speed.toFixed(1)} km/h</div>
                <div><b>Heading:</b> ${v.telemetry.heading.toFixed(1)}°</div>
                <div><b>Fuel:</b> ${v.telemetry.fuelLevel.toFixed(1)}%</div>
                <div><b>Payload:</b> ${v.telemetry.payload.toFixed(1)}t</div>
                <div><b>RPM:</b> ${v.telemetry.engineRpm.toFixed(0)}</div>
                <hr style="border:none; border-top:1px solid #ddd; margin:6px 0;">
                <div style="font-size:11px; opacity:0.5;">
                    ${v.position.latitude.toFixed(6)}, ${v.position.longitude.toFixed(6)}
                </div>
            </div>
        `);
    });
}

// ============================================================
// Atualizar sidebar - Fleet List
// ============================================================

const CYCLE_STATES = ['IDLE', 'LOADING', 'HAULING', 'DUMPING', 'RETURNING'];
const CYCLE_CLASSES = ['idle', 'loading', 'hauling', 'dumping', 'returning'];

function updateFleetList(vehicles) {
    const container = document.getElementById('fleet-list');

    if (vehicles.length === 0) {
        container.innerHTML = '<p class="no-data">Waiting for data...</p>';
        return;
    }

    container.innerHTML = vehicles.map(v => {
        const stateClass = CYCLE_CLASSES[v.cycleState] || 'idle';
        const stateName = CYCLE_STATES[v.cycleState] || 'UNKNOWN';

        return `
            <div class="vehicle-card" onclick="focusVehicle('${v.id}')">
                <div class="vehicle-header">
                    <span class="vehicle-id">${v.id}</span>
                    <span class="vehicle-type">${v.vehicleTypeName}</span>
                </div>
                <span class="vehicle-state state-${stateClass}">${stateName}</span>
                <div class="vehicle-stats">
                    <div class="vehicle-stat">
                        <span class="vehicle-stat-label">Speed</span>
                        <span class="vehicle-stat-value">${v.telemetry.speed.toFixed(1)} km/h</span>
                    </div>
                    <div class="vehicle-stat">
                        <span class="vehicle-stat-label">Heading</span>
                        <span class="vehicle-stat-value">${v.telemetry.heading.toFixed(0)}°</span>
                    </div>
                    <div class="vehicle-stat">
                        <span class="vehicle-stat-label">Fuel</span>
                        <span class="vehicle-stat-value">${v.telemetry.fuelLevel.toFixed(1)}%</span>
                    </div>
                    <div class="vehicle-stat">
                        <span class="vehicle-stat-label">Payload</span>
                        <span class="vehicle-stat-value">${v.telemetry.payload.toFixed(1)}t</span>
                    </div>
                </div>
            </div>
        `;
    }).join('');
}

// Clicar num veiculo na sidebar foca no mapa
function focusVehicle(id) {
    const marker = vehicleMarkers[id];
    if (marker) {
        map.setView(marker.getLatLng(), 17);
        marker.openPopup();
    }
}

// ============================================================
// Atualizar Alert Center
// ============================================================

const PRIORITY_NAMES = ['NONE', 'LOW', 'MEDIUM', 'HIGH', 'CRITICAL'];
const ALERT_TYPES = ['APPROACH', 'CROSSING', 'TAILGATING', 'BLIND_SPOT'];

function updateAlerts(alerts) {
    const container = document.getElementById('alert-list');
    const badge = document.getElementById('alert-count');

    if (alerts.length === 0) {
        container.innerHTML = '<p class="no-data">No active alerts</p>';
        badge.style.display = 'none';
        return;
    }

    // Ordenar por prioridade (maior primeiro)
    alerts.sort((a, b) => b.priority - a.priority);

    badge.textContent = alerts.length;
    badge.style.display = 'inline';

    container.innerHTML = alerts.map(a => {
        const priorityName = PRIORITY_NAMES[a.priority] || 'UNKNOWN';
        const alertType = ALERT_TYPES[a.alertType] || 'UNKNOWN';

        return `
            <div class="alert-item priority-${a.priority}">
                <span class="alert-priority">${priorityName}</span>
                <span class="alert-vehicles">${a.vehicleId1} ↔ ${a.vehicleId2}</span>
                <span class="alert-type">${alertType}</span>
                <span class="alert-tti">${a.timeToImpact.toFixed(1)}s</span>
                <span class="alert-distance">${a.distance.toFixed(1)}m</span>
            </div>
        `;
    }).join('');
}

// ============================================================
// Atualizar header (status, uptime)
// ============================================================

function updateStatus(status) {
    const dot = document.querySelector('#connection-status .status-dot');
    const text = document.querySelector('#connection-status .status-text');

    if (status.simulatorConnected) {
        dot.className = 'status-dot online';
        text.textContent = 'Simulator Connected';
    } else {
        dot.className = 'status-dot offline';
        text.textContent = 'Disconnected';
    }

    // Uptime
    const secs = status.uptimeSeconds;
    const h = Math.floor(secs / 3600);
    const m = Math.floor((secs % 3600) / 60);
    const s = secs % 60;
    document.getElementById('uptime').textContent =
        `${h.toString().padStart(2, '0')}:${m.toString().padStart(2, '0')}:${s.toString().padStart(2, '0')}`;

    // Stats
    document.getElementById('total-vehicles').textContent = status.totalVehicles;
    document.getElementById('online-vehicles').textContent = status.onlineVehicles;
    document.getElementById('active-alerts').textContent = status.activeAlerts;
    document.getElementById('total-alerts').textContent = status.totalAlertsReceived;
}

// ============================================================
// Polling - busca dados da API a cada 1 segundo
// ============================================================

let backendOnline = false;

async function poll() {
    try {
        const [vehiclesRes, alertsRes, statusRes] = await Promise.all([
            fetch(`${API_BASE}/vehicles`),
            fetch(`${API_BASE}/alerts`),
            fetch(`${API_BASE}/status`)
        ]);

        const vehicles = await vehiclesRes.json();
        const alerts = await alertsRes.json();
        const status = await statusRes.json();

        updateMap(vehicles);
        updateFleetList(vehicles);
        updateAlerts(alerts);
        updateStatus(status);

        if (!backendOnline) {
            backendOnline = true;
            console.log('[MineGuard] Connected to backend');
        }
    } catch (err) {
        if (backendOnline) {
            backendOnline = false;
            console.warn('[MineGuard] Backend offline:', err.message);
        }
    }
}

// ============================================================
// Inicializacao
// ============================================================

// Waypoints da mina (pra referencia visual)
const mineWaypoints = [
    { name: 'PIT', pos: [-20.12200, -43.95200] },
    { name: 'RAMP', pos: [-20.12000, -43.95000] },
    { name: 'ROAD', pos: [-20.11750, -43.94750] },
    { name: 'DUMP', pos: [-20.11550, -43.94575] }
];

// Adicionar labels dos waypoints no mapa
mineWaypoints.forEach(wp => {
    L.marker(wp.pos, {
        icon: L.divIcon({
            className: 'waypoint-label',
            html: `<div style="
                color: #5d6280;
                font-size: 10px;
                font-weight: 600;
                font-family: Inter, sans-serif;
                text-shadow: 0 0 4px #0f1117;
                white-space: nowrap;
            ">${wp.name}</div>`,
            iconSize: [60, 16],
            iconAnchor: [30, -8]
        })
    }).addTo(map);
});

// Linha da rota principal (visual)
const routeLine = L.polyline(
    mineWaypoints.map(wp => wp.pos),
    { color: '#363b50', weight: 3, dashArray: '8,6', opacity: 0.5 }
).addTo(map);

// Iniciar polling
setInterval(poll, POLL_INTERVAL);
poll();

console.log('[MineGuard] Dashboard initialized');

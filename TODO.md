# MineGuard - Plano de Desenvolvimento

## Simulador C++

### Etapa 1 - Estrutura do projeto e setup inicial
- Criar pastas (`simulator/include`, `simulator/src`, `backend/MineGuard.Api`, `dashboard/css`, `dashboard/js`, `docs`)
- Criar `CMakeLists.txt` para o simulador
- Criar projeto .NET (`MineGuard.Api.csproj`, `Program.cs`)
- Criar arquivos base vazios (headers, sources)

### Etapa 2 - Estruturas de dados e modelos
- `telemetry.hpp` - Structs de posicao (lat, lon, altitude), telemetria (velocidade, heading, payload, fuel), pacote de dados com timestamp
- `vehicle.hpp` - Enum de tipos de veiculo (HaulTruck, Excavator, LightVehicle), enum de estados do ciclo (Loading, Hauling, Dumping, Returning, Idle), classe Vehicle com atributos e interface publica

### Etapa 3 - Logica do veiculo
- `vehicle.cpp` - Movimentacao baseada em velocidade e heading
- Simulacao de posicao GNSS (coordenadas reais de uma mina)
- Ciclo de producao: Load → Haul → Dump → Return
- Efeitos de fisica simplificada: aceleracao, consumo de combustivel, efeito de payload na velocidade

### Etapa 4 - Fleet Manager
- `fleet.hpp` / `fleet.cpp` - Criacao e gerenciamento da frota (3 haul trucks, 1 excavator, 1 light vehicle)
- Loop de simulacao com update a 1 Hz
- Coleta de telemetria de todos os veiculos

### Etapa 5 - Deteccao de colisao
- `collision.hpp` / `collision.cpp` - Algoritmo de Path Prediction
- Calculo de Closest Point of Approach (CPA)
- Calculo de Time to Impact (TTI)
- Geracao de alertas com prioridade: LOW (10-15s), MEDIUM (5-10s), HIGH (3-5s), CRITICAL (<3s)
- Classificacao: approach, crossing, tailgating, blind spot

### Etapa 6 - Cliente TCP com JSON
- Serializacao de telemetria em JSON
- Envio via TCP com length-prefix (4 bytes + payload)
- Conexao com reconexao automatica
- Buffer de envio

### Etapa 7 - main.cpp
- Parsing de argumentos (`--local`, `--host`, `--port`)
- Modo local: output no console para testes
- Modo rede: conecta ao backend via TCP
- Loop principal integrando fleet manager, collision detection e envio de dados

---

## Backend .NET

### Etapa 8 - Projeto base, Models e TCP Listener
- Models: `Vehicle`, `TelemetryPacket`, `CollisionAlert`, `SystemStatus`
- TCP Listener como Background Service (porta 5000)
- Parse de JSON com length-prefix
- Validacao dos pacotes recebidos

### Etapa 9 - Services
- `FleetService` - Estado atual de todos os veiculos em memoria
- `AlertService` - Processamento e armazenamento de alertas de colisao
- `TelemetryService` - Ingestao e processamento de telemetria recebida

### Etapa 10 - REST API Controllers
- `GET /api/vehicles` - Lista de veiculos com posicao e status atual
- `GET /api/vehicles/{id}` - Detalhes de um veiculo especifico
- `GET /api/alerts` - Alertas de colisao ativos
- `GET /api/alerts/history` - Historico de alertas com filtros
- `GET /api/status` - Saude do sistema e estatisticas

---

## Dashboard Web

### Etapa 11 - HTML base + CSS
- Layout do painel: header, sidebar, area principal
- Cards de veiculos com status e telemetria
- Painel de alertas
- Design responsivo e tema escuro (estilo industrial/mining)

### Etapa 12 - Mapa interativo com Leaflet.js
- Mapa centrado nas coordenadas da mina simulada
- Icones por tipo de veiculo (truck, excavator, light vehicle)
- Atualizacao de posicoes em tempo real
- Raio de seguranca visual ao redor dos veiculos

### Etapa 13 - Fleet Status + Alert Center
- Polling da REST API a cada 1 segundo
- Cards de veiculos com velocidade, fuel, payload
- Centro de alertas com alertas ativos e historico
- Indicadores visuais de prioridade (cores por nivel)

---

## Integracao

### Etapa 14 - Testes end-to-end
- Validar fluxo completo: Simulador C++ → TCP → Backend .NET → REST API → Dashboard
- Testar cenarios de colisao e geracao de alertas
- Verificar reconexao TCP
- Teste de performance com frota completa

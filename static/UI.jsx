const UI = () => {
    const [playerData, setPlayerData] = React.useState([]);
    const [selectedPlayer, setSelectedPlayer] = React.useState(null);
    const [selectedMap, setSelectedMap] = React.useState(null);
    const [availableMaps, setAvailableMaps] = React.useState([]);
    const [mapConfig, setMapConfig] = React.useState(null);
    const [currentSection, setCurrentSection] = React.useState("default");
    const [availableSections, setAvailableSections] = React.useState([]);
    const [connectionStatus, setConnectionStatus] = React.useState('connecting');
    const [mapImage, setMapImage] = React.useState(null);
    const [mapOnlyMode, setMapOnlyMode] = React.useState(false);

    const canvasRef = React.useRef(null);
    const canvasContainerRef = React.useRef(null);

    React.useEffect(() => {
        let ws = null;
        let reconnectTimeout = null;
        const connectWebSocket = () => {
            setConnectionStatus('connecting');
            if (reconnectTimeout) {
                clearTimeout(reconnectTimeout);
                reconnectTimeout = null;
            }
            ws = new WebSocket(`ws://${window.location.hostname}:8080/ws`);
            ws.onopen = () => {
                setConnectionStatus('connected');
                console.log('WebSocket connection established');
            };
            ws.onmessage = (event) => {
                try {
                    if (typeof event.data !== 'string') {
                        console.warn("Received non-text message, skipping processing");
                        return;
                    }

                    const message = JSON.parse(event.data);
                    if (message.type === "maps_list") {
                        if (availableMaps.length) {
                            console.warn("Received maps list while already having one, ignoring");
                            return;
                        }
                        setAvailableMaps(message.maps);
                        if (message.maps.length > 0) {
                            if (!selectedMap) {
                                setSelectedMap(message.maps[0].id);
                                setMapConfig(message.maps[0]);
                            }
                        }
                        return;
                    }
                    if (message.type === "player_data") {
                        setPlayerData(message.data);
                        return;
                    }
                    console.warn("Unknown message type:", message.type);
                } catch (error) {
                    console.error("Error processing WebSocket message:", error);
                }
            };
            ws.onclose = (event) => {
                setConnectionStatus('disconnected');

                console.log(`WebSocket closed with code ${event.code}, reason: ${event.reason || 'No reason provided'}`);

                console.log("Attempting to reconnect in 3 seconds...");
                reconnectTimeout = setTimeout(connectWebSocket, 3000);
            };
            ws.onerror = (error) => {
                setConnectionStatus('error');
                console.error("WebSocket error:", error);
            };
        };
        connectWebSocket();
        return () => {
            if (ws) {
                ws.close();
            }
            if (reconnectTimeout) {
                clearTimeout(reconnectTimeout);
            }
        };
    }, []);

    React.useEffect(() => {
        if (!selectedMap || !availableMaps.length) return;

        const newConfig = availableMaps.find(m => m.id === selectedMap);
        if (newConfig) {
            setMapConfig(newConfig);
        }
    }, [selectedMap, availableMaps]);

    const handlePlayerClick = (player) => {
        if (player === selectedPlayer) {
            setSelectedPlayer(null);
            return;
        }
        setSelectedPlayer(player);
    };

    const handleMapChange = (e) => {
        setSelectedMap(e.target.value);
    };

    const handleSectionChange = (e) => {
        setCurrentSection(e.target.value);
    };

    const toggleMapOnlyMode = () => {
        setMapOnlyMode(!mapOnlyMode);
    };

    return (
        <div className="flex flex-col h-screen p-4 bg-gray-900 text-white overflow-hidden">
            <div className="flex justify-between items-center mb-2">
                <div className="flex items-center">
                    <div className="flex items-center mr-4">
                        <div className={`w-3 h-3 rounded-full mr-2 ${connectionStatus === 'connected' ? 'bg-green-500' :
                            connectionStatus === 'connecting' ? 'bg-yellow-500' :
                                'bg-red-500'
                            }`}></div>
                        <span className="text-sm">
                            {connectionStatus === 'connected' ? 'Connected' :
                                connectionStatus === 'connecting' ? 'Connecting...' :
                                    'Disconnected'}
                        </span>
                    </div>

                    <div className="flex items-center">
                        <select
                            className="bg-gray-800 text-white py-1 px-3 rounded mr-2"
                            value={selectedMap || ''}
                            onChange={handleMapChange}
                        >
                            {availableMaps.map(map => (
                                <option key={map.id} value={map.id}>
                                    {map.name}
                                </option>
                            ))}
                        </select>
                    </div>

                    <button
                        className={`py-1 px-3 rounded mr-2 ${mapOnlyMode ? 'bg-blue-600' : 'bg-gray-800'}`}
                        onClick={toggleMapOnlyMode}
                    >
                        {mapOnlyMode ? 'Show All' : 'Map Only'}
                    </button>

                </div>
                <a href="https://github.com/0mlml/munakas">GitHub</a>
            </div>

            <div className="flex flex-row h-full gap-4">
                <MapComponent
                    mapConfig={mapConfig}
                    selectedMap={selectedMap}
                    setMapImage={setMapImage}
                    setAvailableSections={setAvailableSections}
                    setCurrentSection={setCurrentSection}
                    currentSection={currentSection}
                    selectedPlayer={selectedPlayer}
                    playerData={playerData}
                    canvasRef={canvasRef}
                    canvasContainerRef={canvasContainerRef}
                    setPlayerData={setPlayerData}
                    mapImage={mapImage}
                    setSelectedPlayer={setSelectedPlayer}
                    mapOnlyMode={mapOnlyMode}
                    availableSections={availableSections}
                />

                {!mapOnlyMode && (
                    <div className="w-1/3 bg-gray-800 rounded-lg p-4 overflow-y-auto">
                        <h2 className="text-xl font-semibold mb-2">Players</h2>
                        <div className="space-y-2">
                            {playerData.map(player => (
                                <div
                                    key={player.name}
                                    className={`relative p-2 rounded-md cursor-pointer ${selectedPlayer && player.name === selectedPlayer.name ? 'ring-2 ring-white' : ''} ${getTeamColor(player.team)}`}
                                    onClick={() => handlePlayerClick(player)}
                                >
                                    <div className="flex justify-between items-center select-none">
                                        <div className="font-bold flex items-center">
                                            {player.active_player && <span className="mr-1">👁️</span>}
                                            {player.name}
                                            {!(!mapConfig.verticalSections ||
                                                !mapConfig.verticalSections[currentSection] ||
                                                (player.position.z >= mapConfig.verticalSections[currentSection].altitudeMin &&
                                                    player.position.z < mapConfig.verticalSections[currentSection].altitudeMax)) && player.life_state === 0 &&
                                                <span className="ml-1 text-xs">(different level)</span>
                                            }
                                        </div>
                                        <div className="text-sm">
                                            ${player.money}
                                        </div>
                                    </div>

                                    <div className="flex items-center mt-1">
                                        <div className="w-20 bg-gray-700 h-3 rounded-sm mr-2">
                                            <div
                                                className={`h-3 rounded-sm ${player.health > 60 ? 'bg-green-500' :
                                                    player.health > 20 ? 'bg-yellow-500' : 'bg-red-500'
                                                    }`}
                                                style={{ width: `${player.health}%` }}
                                            />
                                        </div>

                                        {player.armor > 0 && (
                                            <div className="w-5 h-5 bg-gray-300 rounded-full flex items-center justify-center text-gray-800 text-xs select-none">
                                                {player.armor}
                                            </div>
                                        )}

                                        <div className="ml-2 text-xs select-none">
                                            {getWeaponName(player.weapon)}
                                        </div>
                                    </div>

                                    {player.life_state !== 0 && (
                                        <div className="absolute inset-0 bg-black bg-opacity-70 rounded-md flex items-center justify-center">
                                            <span className="text-red-500 font-bold text-lg">DEAD</span>
                                        </div>
                                    )}
                                </div>
                            ))}
                        </div>
                    </div>
                )}
            </div>
        </div >
    );
};
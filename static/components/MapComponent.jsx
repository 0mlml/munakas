const MapComponent = ({
    selectedMap,
    mapConfig,
    setMapImage,
    setAvailableSections,
    setCurrentSection,
    currentSection,
    selectedPlayer,
    playerData,
    canvasRef,
    canvasContainerRef,
    setPlayerData,
    mapImage,
    setSelectedPlayer,
    mapOnlyMode,
    availableSections,
    bombState,
}) => {
    React.useEffect(() => {
        if (!mapConfig || !mapConfig.verticalSections) {
            setAvailableSections(["default"]);
            setCurrentSection("default");
            return;
        }

        const sections = Object.keys(mapConfig.verticalSections);
        setAvailableSections(sections);

        if (sections.includes("default")) {
            setCurrentSection("default");
        } else if (sections.length > 0) {
            setCurrentSection(sections[0]);
        }
    }, [mapConfig]);

    React.useEffect(() => {
        if (!selectedMap) return;

        const img = new Image();

        const sectionSuffix = currentSection !== "default" ? `_${currentSection}` : '';
        img.src = `/cs2-radar-images/${selectedMap}${sectionSuffix}.png`;

        img.onload = () => {
            setMapImage(img);
        };
        img.onerror = () => {
            console.error(`Failed to load image: ${img.src}`);
            const defaultImg = new Image();
            defaultImg.src = '/cs2-radar-images/default.png';
            defaultImg.onload = () => {
                setMapImage(defaultImg);
            };
        };
    }, [selectedMap, currentSection]);

    React.useEffect(() => {
        if (!selectedPlayer || !mapConfig || !mapConfig.verticalSections) return;

        const playerZ = selectedPlayer.position.z;

        let newSection = null;
        let highestMinimum = -Infinity;

        Object.entries(mapConfig.verticalSections).forEach(([sectionName, sectionConfig]) => {
            if (playerZ >= sectionConfig.altitudeMin &&
                playerZ < sectionConfig.altitudeMax &&
                sectionConfig.altitudeMin > highestMinimum) {
                newSection = sectionName;
                highestMinimum = sectionConfig.altitudeMin;
            }
        });

        if (newSection && newSection !== currentSection) {
            setCurrentSection(newSection);
        }
    }, [selectedPlayer, mapConfig]);

    React.useEffect(() => {
        if (!canvasRef.current || !mapImage || !mapConfig) return;

        const canvas = canvasRef.current;
        const ctx = canvas.getContext('2d');

        const container = canvasContainerRef.current;
        if (!container) return;

        const containerWidth = container.clientWidth;
        const containerHeight = container.clientHeight;

        const dpr = window.devicePixelRatio || 1;
        canvas.width = containerWidth * dpr;
        canvas.height = containerHeight * dpr;

        ctx.scale(dpr, dpr);

        ctx.clearRect(0, 0, containerWidth, containerHeight);

        const mapAspectRatio = mapImage.width / mapImage.height;
        let drawWidth, drawHeight, drawX, drawY;

        if (containerWidth / containerHeight > mapAspectRatio) {
            drawHeight = containerHeight;
            drawWidth = drawHeight * mapAspectRatio;
            drawX = (containerWidth - drawWidth) / 2;
            drawY = 0;
        } else {
            drawWidth = containerWidth;
            drawHeight = drawWidth / mapAspectRatio;
            drawX = 0;
            drawY = (containerHeight - drawHeight) / 2;
        }

        ctx.save();

        if (mapConfig.rotate) {
            ctx.translate(containerWidth / 2, containerHeight / 2);

            const rotationAngle = mapConfig.rotate === 0 ? Math.PI / 2 :
                mapConfig.rotate === 1 ? Math.PI : 0;
            ctx.rotate(rotationAngle);

            if (mapConfig.rotate === 0 || mapConfig.rotate === 1) {
                const temp = drawWidth;
                drawWidth = drawHeight;
                drawHeight = temp;
            }

            ctx.drawImage(mapImage, -drawWidth / 2, -drawHeight / 2, drawWidth, drawHeight);
        } else {
            ctx.drawImage(mapImage, drawX, drawY, drawWidth, drawHeight);
        }

        ctx.restore();

        playerData.forEach(player => {
            if (player.lifeState !== 0) return;

            const renderTranslucent = mapConfig.verticalSections && mapConfig.verticalSections[currentSection] &&
                (player.position.z < mapConfig.verticalSections[currentSection].altitudeMin ||
                    player.position.z >= mapConfig.verticalSections[currentSection].altitudeMax);

            const position = calculatePlayerCanvasPosition(player, drawX, drawY, drawWidth, drawHeight);

            ctx.beginPath();
            ctx.arc(position.x, position.y, window.innerWidth < 768 ? 4 : 8, 0, 2 * Math.PI);

            switch (player.team) {
                case 2: ctx.fillStyle = `rgba(202, 138, 4, ${renderTranslucent ? 0.5 : 1})`; break
                case 3: ctx.fillStyle = `rgba(37, 99, 235, ${renderTranslucent ? 0.5 : 1})`; break
                default: ctx.fillStyle = `rgba(75, 85, 99, ${renderTranslucent ? 0.5 : 1})`; break
            }

            ctx.fill();

            if (selectedPlayer && player.name === selectedPlayer.name) {

                ctx.strokeStyle = 'white';
                ctx.lineWidth = 2;
                ctx.stroke();
                let newSection = null;
                let highestMinimum = -Infinity;
                if (mapConfig.verticalSections) Object.entries(mapConfig.verticalSections).forEach(([sectionName, sectionConfig]) => {
                    if (player.position.z >= sectionConfig.altitudeMin &&
                        player.position.z < sectionConfig.altitudeMax &&
                        sectionConfig.altitudeMin > highestMinimum) {
                        newSection = sectionName;
                        highestMinimum = sectionConfig.altitudeMin;
                    }
                });

                if (newSection && newSection !== currentSection) {
                    setCurrentSection(newSection);
                }
            }

            const radius = window.innerWidth < 768 ? 12 : 20; // Distance from player center
            const triangleSize = window.innerWidth < 768 ? 5 : 8; // Size of the triangle

            const yawRadians = (-player.yaw * Math.PI) / 180;

            const triTipX = position.x + Math.cos(yawRadians) * radius;
            const triTipY = position.y + Math.sin(yawRadians) * radius;

            const triBase1X = position.x + Math.cos(yawRadians + Math.PI * 0.8) * triangleSize;
            const triBase1Y = position.y + Math.sin(yawRadians + Math.PI * 0.8) * triangleSize;

            const triBase2X = position.x + Math.cos(yawRadians - Math.PI * 0.8) * triangleSize;
            const triBase2Y = position.y + Math.sin(yawRadians - Math.PI * 0.8) * triangleSize;

            ctx.beginPath();
            ctx.moveTo(triTipX, triTipY);
            ctx.lineTo(triBase1X, triBase1Y);
            ctx.lineTo(triBase2X, triBase2Y);
            ctx.closePath();
            ctx.fill();

            if (player.hasBomb) {
                ctx.fillStyle = 'red';
                ctx.beginPath();
                ctx.arc(position.x, position.y, window.innerWidth < 768 ? 2 : 4, 0, 2 * Math.PI);
                ctx.fill();
            }

            ctx.font = '12px Arial';
            ctx.fillStyle = 'white';
            ctx.textAlign = 'center';
            ctx.fillText(player.name, position.x, position.y + 20);
        });

    }, [playerData, mapImage, mapConfig, currentSection, selectedPlayer]);


    React.useEffect(() => {
        const handleResize = () => {
            if (!canvasRef.current || !mapImage || !mapConfig) return;

            const canvas = canvasRef.current;
            const container = canvasContainerRef.current;
            if (!container) return;

            const containerWidth = container.clientWidth;
            const containerHeight = container.clientHeight;

            const dpr = window.devicePixelRatio || 1;
            canvas.width = containerWidth * dpr;
            canvas.height = containerHeight * dpr;

            setPlayerData([...playerData]);
        };

        window.addEventListener('resize', handleResize);
        return () => window.removeEventListener('resize', handleResize);
    }, [playerData, mapImage, mapConfig]);

    const calculatePlayerCanvasPosition = (player, drawX, drawY, drawWidth, drawHeight) => {
        if (!mapConfig) {
            return { x: drawX + drawWidth / 2, y: drawY + drawHeight / 2 };
        }

        const scale = 1 / mapConfig.scale;

        let gameX = (player.position.x - mapConfig.pos_x) * scale;
        let gameY = -(player.position.y - mapConfig.pos_y) * scale;

        gameX = gameX / 1024;
        gameY = gameY / 1024;

        let canvasX = drawX + (gameX * drawWidth);
        let canvasY = drawY + (gameY * drawHeight);

        if (mapConfig.rotate) {
            const centerX = drawX + drawWidth / 2;
            const centerY = drawY + drawHeight / 2;

            const translatedX = canvasX - centerX;
            const translatedY = canvasY - centerY;

            let rotatedX, rotatedY;
            if (mapConfig.rotate === 0) {
                rotatedX = -translatedY;
                rotatedY = translatedX;
            } else if (mapConfig.rotate === 1) {
                rotatedX = -translatedX;
                rotatedY = -translatedY;
            } else {
                rotatedX = translatedX;
                rotatedY = translatedY;
            }

            canvasX = centerX + rotatedX;
            canvasY = centerY + rotatedY;
        }

        return {
            x: canvasX,
            y: canvasY
        };
    };

    const handleCanvasClick = (e) => {
        if (!canvasRef.current || !playerData.length) return;

        const canvas = canvasRef.current;
        const rect = canvas.getBoundingClientRect();
        const x = e.clientX - rect.left;
        const y = e.clientY - rect.top;

        let closestPlayer = null;
        let closestDistance = Infinity;

        playerData.forEach(player => {
            if (player.lifeState !== 0) return;

            if (mapConfig.verticalSections) {
                const section = mapConfig.verticalSections[currentSection];
                if (section && (player.position.z < section.altitudeMin || player.position.z >= section.altitudeMax)) {
                    return;
                }
            }

            const container = canvasContainerRef.current;
            if (!container) return;

            const containerWidth = container.clientWidth;
            const containerHeight = container.clientHeight;

            const mapAspectRatio = mapImage ? mapImage.width / mapImage.height : 1;
            let drawWidth, drawHeight, drawX, drawY;

            if (containerWidth / containerHeight > mapAspectRatio) {
                drawHeight = containerHeight;
                drawWidth = drawHeight * mapAspectRatio;
                drawX = (containerWidth - drawWidth) / 2;
                drawY = 0;
            } else {
                drawWidth = containerWidth;
                drawHeight = drawWidth / mapAspectRatio;
                drawX = 0;
                drawY = (containerHeight - drawHeight) / 2;
            }

            const position = calculatePlayerCanvasPosition(player, drawX, drawY, drawWidth, drawHeight);
            const distance = Math.sqrt(Math.pow(position.x - x, 2) + Math.pow(position.y - y, 2));

            if (distance < closestDistance) {
                closestDistance = distance;
                closestPlayer = player;
            }
        });

        if (closestDistance < 20 && closestPlayer) {
            setSelectedPlayer(closestPlayer);
        }
    };

    return (
        <div className={`${mapOnlyMode ? 'w-full' : 'w-2/3'} bg-gray-800 rounded-lg p-4 flex flex-col`}>
            {!mapOnlyMode && (<h2 className="text-xl font-semibold mb-2">
                Map - {selectedMap && mapConfig ? mapConfig.name : ""}
                {availableSections.length > 1 && ` (${currentSection.charAt(0).toUpperCase() + currentSection.slice(1)} Level)`}
            </h2>)}
            <div ref={canvasContainerRef} className="relative flex-grow bg-gray-700 rounded overflow-hidden">
                <canvas
                    ref={canvasRef}
                    onClick={handleCanvasClick}
                    className="cursor-pointer"
                />
            </div>

            {!mapOnlyMode && selectedPlayer ? (
                <div className="bg-gray-700 rounded-lg p-2 mt-4">
                    <h3 className="font-semibold mb-1 flex items-center">
                        <span className={`w-2 h-2 rounded-full ${getTeamColor(selectedPlayer.team)} mr-2`}></span>
                        {selectedPlayer.name} ({getTeamName(selectedPlayer.team)})
                    </h3>

                    <div className="grid grid-cols-3 gap-2 text-sm">
                        <div>
                            <span className="text-gray-400">Health:</span> {selectedPlayer.health}
                        </div>
                        <div>
                            <span className="text-gray-400">Armor:</span> {selectedPlayer.armor}
                        </div>
                        <div>
                            <span className="text-gray-400">Money:</span> ${selectedPlayer.money}
                        </div>
                        <div>
                            <span className="text-gray-400">Weapons:</span> {selectedPlayer.getWeapons()}
                        </div>
                        <div>
                            <span className="text-gray-400">X:</span> {selectedPlayer.position.x.toFixed(0)}
                        </div>
                        <div>
                            <span className="text-gray-400">Y:</span> {selectedPlayer.position.y.toFixed(0)}
                        </div>
                        <div>
                            <span className="text-gray-400">Z:</span> {selectedPlayer.position.z.toFixed(0)}
                        </div>
                    </div>
                </div>
            ) :
                !mapOnlyMode && (
                    <div className="bg-gray-700 rounded-lg p-2 mt-4">
                        <div className="grid grid-cols-3 gap-2 text-sm">
                            <h3 className="font-semibold mb-1 flex items-center">
                                <span className="w-2 h-2 rounded-full bg-yellow-600 mr-2"></span>
                                {playerData.filter(player => player.team === 2 && player.lifeState === 0).length} Terrorists
                            </h3>
                            <h4>vs</h4>
                            <h3 className="font-semibold mb-1 flex items-center">
                                <span className="w-2 h-2 rounded-full bg-blue-600 mr-2"></span>
                                {playerData.filter(player => player.team === 3 && player.lifeState === 0).length} Counter-Terrorists
                            </h3>
                        </div>
                    </div>
                )}
        </div>
    )
}
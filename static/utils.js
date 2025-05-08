const getTeamName = (teamId) => {
    switch (teamId) {
        case 2: return "Terrorists";
        case 3: return "Counter-Terrorists";
        default: return "Spectator";
    }
};

const getTeamColor = (teamId) => {
    switch (teamId) {
        case 2: return "bg-yellow-600";
        case 3: return "bg-blue-600";
        default: return "bg-gray-600";
    }
};

const getWeaponName = (weaponId) => {
    return weaponId.replace('weapon_', '');
};
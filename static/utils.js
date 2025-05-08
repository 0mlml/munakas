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

const weaponClasses = {
    primary: [
        "weapon_m4a1_silencer",
        "weapon_scar20",
        "weapon_sg556",
        "weapon_ssg08",
        "weapon_mp7",
        "weapon_mp9",
        "weapon_nova",
        "weapon_ak47",
        "weapon_aug",
        "weapon_awp",
        "weapon_famas",
        "weapon_g3sg1",
        "weapon_galilar",
        "weapon_m249",
        "weapon_m4a1",
        "weapon_mac10",
        "weapon_p90",
        "weapon_ump",
        "weapon_xm1014",
        "weapon_bizon",
        "weapon_mag7",
        "weapon_negev",
        "weapon_sawedoff",
    ],
    secondary: [
        "weapon_usp_silencer",
        "weapon_cz75a",
        "weapon_revolver",
        "weapon_p250",
        "weapon_hkp2000",
        "weapon_deagle",
        "weapon_elite",
        "weapon_fiveseven",
        "weapon_glock",
        "weapon_tec9",
    ],
    knife: [
        "weapon_knife",
        "weapon_knife_t",
        "weapon_bayonet",
        "weapon_knife_flip",
        "weapon_knife_gut",
        "weapon_knife_karambit",
        "weapon_knife_m9_bayonet",
        "weapon_knife_tactical",
        "weapon_knife_falchion",
        "weapon_knife_survival_bowie",
        "weapon_knife_butterfly",
        "weapon_knife_push",
        "weapon_knife_kukri"
    ],
    misc: [
        "weapon_c4",
        "weapon_flashbang",
        "weapon_hegrenade",
        "weapon_smokegrenade",
        "weapon_molotov",
        "weapon_decoy",
        "weapon_incgrenade",
        "weapon_taser",
    ],
}

const weaponDisplayNames = {
    "weapon_m4a1_silencer": "M4A1-S",
    "weapon_scar20": "SCAR-20",
    "weapon_sg556": "SG 553",
    "weapon_ssg08": "SSG 08",
    "weapon_mp7": "MP7",
    "weapon_mp9": "MP9",
    "weapon_nova": "Nova",
    "weapon_ak47": "AK-47",
    "weapon_aug": "AUG",
    "weapon_awp": "AWP",
    "weapon_famas": "FAMAS",
    "weapon_g3sg1": "G3SG1",
    "weapon_galilar": "Galil AR",
    "weapon_m249": "M249",
    "weapon_m4a1": "M4A4",
    "weapon_mac10": "MAC-10",
    "weapon_p90": "P90",
    "weapon_ump": "UMP-45",
    "weapon_xm1014": "XM1014",
    "weapon_bizon": "PP-Bizon",
    "weapon_mag7": "MAG-7",
    "weapon_negev": "Negev",
    "weapon_sawedoff": "Sawed-Off",

    "weapon_usp_silencer": "USP-S",
    "weapon_cz75a": "CZ75-Auto",
    "weapon_revolver": "R8 Revolver",
    "weapon_p250": "P250",
    "weapon_hkp2000": "P2000",
    "weapon_deagle": "Desert Eagle",
    "weapon_elite": "Dual Berettas",
    "weapon_fiveseven": "Five-SeveN",
    "weapon_glock": "Glock-18",
    "weapon_tec9": "Tec-9",

    "weapon_knife": "Knife",
    "weapon_knife_t": "Knife",
    "weapon_bayonet": "Bayonet",
    "weapon_knife_flip": "Flip Knife",
    "weapon_knife_gut": "Gut Knife",
    "weapon_knife_karambit": "Karambit",
    "weapon_knife_m9_bayonet": "M9 Bayonet",
    "weapon_knife_tactical": "Huntsman Knife",
    "weapon_knife_falchion": "Falchion Knife",
    "weapon_knife_survival_bowie": "Bowie Knife",
    "weapon_knife_butterfly": "Butterfly Knife",
    "weapon_knife_push": "Shadow Daggers",
    "weapon_knife_kukri": "Kukri Knife",

    "weapon_c4": "C4 Explosive",
    "weapon_flashbang": "Flashbang",
    "weapon_hegrenade": "HE Grenade",
    "weapon_smokegrenade": "Smoke Grenade",
    "weapon_molotov": "Molotov",
    "weapon_decoy": "Decoy Grenade",
    "weapon_incgrenade": "Incendiary Grenade",
    "weapon_taser": "Zeus x27"
};

const getWeaponClass = (weaponId) => {
    return Object.keys(weaponClasses).find((key) => {
        return weaponClasses[key].includes(weaponId);
    }) || "unknown";
}

const getWeaponName = (weaponId) => {
    return weaponDisplayNames[weaponId] || "Unknown Weapon";
}

class Player {
    constructor(data) {
        this.name = data.name;
        this.health = data.health;
        this.armor = data.armor;
        this.money = data.money;
        this.team = data.team;
        this.lifeState = data.life_state;
        this.weapon = data.weapon;
        this.weapons = data.weapons;
        this.hasBomb = data.has_bomb;
        this.hasDefuser = data.has_defuser;
        this.color = data.color;
        this.position = data.position;
        this.eyeAngles = data.eye_angles;
        this.yaw = data.eye_angles.y;
        this.isActive = data.active_player;
    }

    getPrimaryWeapon() {
        return this.weapons.find(weapon => weaponClasses.primary.includes(weapon));
    }

    getSecondaryWeapon() {
        return this.weapons.find(weapon => weaponClasses.secondary.includes(weapon));
    }

    getWeaponString() {
        const primary = this.getPrimaryWeapon();
        const secondary = this.getSecondaryWeapon();
        return (primary ? getWeaponName(primary) + (secondary ? " / " : "") : "") +
            (secondary ? getWeaponName(secondary) : "");
    }

    getWeapons() {
        return this.weapons.map(getWeaponName).join(", ");
    }
}

window.Player = Player;


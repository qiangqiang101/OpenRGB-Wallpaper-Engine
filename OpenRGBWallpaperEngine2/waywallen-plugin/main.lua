local M = {}

local wallpaper = {}

function wallpaper.properties()
    return {
        ["openrgb.udp_port"] = {
            text = "UDP port",
            type = "u32",
            value = 8133,
        },
        ["openrgb.background_image"] = {
            text = "Background image",
            type = "string",
            value = "",
        },
        ["openrgb.background_color"] = {
            text = "Background color",
            type = "string",
            value = "1,1,1,1",
        },
    }
end

function wallpaper.extras(entry)
    return {
        udp_port = entry.udp_port or 8133,
        background_image = entry.background_image or "",
        background_color = entry.background_color or "1,1,1,1",
    }
end

function M.info()
    return {
        name = "openrgb",
        capabilities = {
            wallpaper = {
                properties = true,
                extras = true,
            },
        },
    }
end

M.wallpaper = wallpaper

return M

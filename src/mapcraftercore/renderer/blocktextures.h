#ifndef BLOCKTEXTURES_H_
#define BLOCKTEXTURES_H_

#include "textureimage.h"

#include <string>
#include <vector>

namespace mapcrafter {
namespace renderer {

/**
 * Collection of Minecraft block textures.
 */
class BlockTextures {
public:
	BlockTextures();
	~BlockTextures();

	bool load(const std::string& block_dir, int size, int blur, double water_opacity);

	TextureImage
		ANVIL_BASE,
		ANVIL_TOP_DAMAGED_0,
		ANVIL_TOP_DAMAGED_1,
		ANVIL_TOP_DAMAGED_2,
		BEACON,
		BED_FEET_END,
		BED_FEET_SIDE,
		BED_FEET_TOP,
		BED_HEAD_END,
		BED_HEAD_SIDE,
		BED_HEAD_TOP,
		BEDROCK,
		BEETROOTS_STAGE_0,
		BEETROOTS_STAGE_1,
		BEETROOTS_STAGE_2,
		BEETROOTS_STAGE_3,
		BONE_BLOCK_SIDE,
		BONE_BLOCK_TOP,
		BOOKSHELF,
		BREWING_STAND,
		BREWING_STAND_BASE,
		BRICK,
		CACTUS_BOTTOM,
		CACTUS_SIDE,
		CACTUS_TOP,
		CAKE_BOTTOM,
		CAKE_INNER,
		CAKE_SIDE,
		CAKE_TOP,
		CARROTS_STAGE_0,
		CARROTS_STAGE_1,
		CARROTS_STAGE_2,
		CARROTS_STAGE_3,
		CAULDRON_BOTTOM,
		CAULDRON_INNER,
		CAULDRON_SIDE,
		CAULDRON_TOP,
		CHAIN_COMMAND_BLOCK_BACK,
		CHAIN_COMMAND_BLOCK_CONDITIONAL,
		CHAIN_COMMAND_BLOCK_FRONT,
		CHAIN_COMMAND_BLOCK_SIDE,
		CHORUS_FLOWER,
		CHORUS_FLOWER_DEAD,
		CHORUS_PLANT,
		CLAY,
		COAL_BLOCK,
		COAL_ORE,
		COARSE_DIRT,
		COBBLESTONE,
		COBBLESTONE_MOSSY,
		COCOA_STAGE_0,
		COCOA_STAGE_1,
		COCOA_STAGE_2,
		COMMAND_BLOCK_BACK,
		COMMAND_BLOCK_CONDITIONAL,
		COMMAND_BLOCK_FRONT,
		COMMAND_BLOCK_SIDE,
		COMPARATOR_OFF,
		COMPARATOR_ON,
		CONCRETE_BLACK,
        CONCRETE_BLUE,
        CONCRETE_BROWN,
        CONCRETE_CYAN,
        CONCRETE_GRAY,
        CONCRETE_GREEN,
        CONCRETE_LIGHT_BLUE,
        CONCRETE_LIME,
        CONCRETE_MAGENTA,
        CONCRETE_ORANGE,
        CONCRETE_PINK,
        CONCRETE_POWDER_BLACK,
        CONCRETE_POWDER_BLUE,
        CONCRETE_POWDER_BROWN,
        CONCRETE_POWDER_CYAN,
        CONCRETE_POWDER_GRAY,
        CONCRETE_POWDER_GREEN,
        CONCRETE_POWDER_LIGHT_BLUE,
        CONCRETE_POWDER_LIME,
        CONCRETE_POWDER_MAGENTA,
        CONCRETE_POWDER_ORANGE,
        CONCRETE_POWDER_PINK,
        CONCRETE_POWDER_PURPLE,
        CONCRETE_POWDER_RED,
        CONCRETE_POWDER_SILVER,
        CONCRETE_POWDER_WHITE,
        CONCRETE_POWDER_YELLOW,
        CONCRETE_PURPLE,
        CONCRETE_RED,
        CONCRETE_SILVER,
        CONCRETE_WHITE,
        CONCRETE_YELLOW,
		CRAFTING_TABLE_FRONT,
		CRAFTING_TABLE_SIDE,
		CRAFTING_TABLE_TOP,
		DAYLIGHT_DETECTOR_INVERTED_TOP,
		DAYLIGHT_DETECTOR_SIDE,
		DAYLIGHT_DETECTOR_TOP,
		DEADBUSH,
		DEBUG,
		DEBUG2,
		DESTROY_STAGE_0,
		DESTROY_STAGE_1,
		DESTROY_STAGE_2,
		DESTROY_STAGE_3,
		DESTROY_STAGE_4,
		DESTROY_STAGE_5,
		DESTROY_STAGE_6,
		DESTROY_STAGE_7,
		DESTROY_STAGE_8,
		DESTROY_STAGE_9,
		DIAMOND_BLOCK,
		DIAMOND_ORE,
		DIRT,
		DIRT_PODZOL_SIDE,
		DIRT_PODZOL_TOP,
		DISPENSER_FRONT_HORIZONTAL,
		DISPENSER_FRONT_VERTICAL,
		DOOR_ACACIA_LOWER,
		DOOR_ACACIA_UPPER,
		DOOR_BIRCH_LOWER,
		DOOR_BIRCH_UPPER,
		DOOR_DARK_OAK_LOWER,
		DOOR_DARK_OAK_UPPER,
		DOOR_IRON_LOWER,
		DOOR_IRON_UPPER,
		DOOR_JUNGLE_LOWER,
		DOOR_JUNGLE_UPPER,
		DOOR_SPRUCE_LOWER,
		DOOR_SPRUCE_UPPER,
		DOOR_WOOD_LOWER,
		DOOR_WOOD_UPPER,
		DOUBLE_PLANT_FERN_BOTTOM,
		DOUBLE_PLANT_FERN_TOP,
		DOUBLE_PLANT_GRASS_BOTTOM,
		DOUBLE_PLANT_GRASS_TOP,
		DOUBLE_PLANT_PAEONIA_BOTTOM,
		DOUBLE_PLANT_PAEONIA_TOP,
		DOUBLE_PLANT_ROSE_BOTTOM,
		DOUBLE_PLANT_ROSE_TOP,
		DOUBLE_PLANT_SUNFLOWER_BACK,
		DOUBLE_PLANT_SUNFLOWER_BOTTOM,
		DOUBLE_PLANT_SUNFLOWER_FRONT,
		DOUBLE_PLANT_SUNFLOWER_TOP,
		DOUBLE_PLANT_SYRINGA_BOTTOM,
		DOUBLE_PLANT_SYRINGA_TOP,
		DRAGON_EGG,
		DROPPER_FRONT_HORIZONTAL,
		DROPPER_FRONT_VERTICAL,
		EMERALD_BLOCK,
		EMERALD_ORE,
		ENCHANTING_TABLE_BOTTOM,
		ENCHANTING_TABLE_SIDE,
		ENCHANTING_TABLE_TOP,
		END_BRICKS,
		END_ROD,
		END_STONE,
		ENDFRAME_EYE,
		ENDFRAME_SIDE,
		ENDFRAME_TOP,
		FARMLAND_DRY,
		FARMLAND_WET,
		FERN,
		FIRE_LAYER_0,
		FIRE_LAYER_1,
		FLOWER_ALLIUM,
		FLOWER_BLUE_ORCHID,
		FLOWER_DANDELION,
		FLOWER_HOUSTONIA,
		FLOWER_OXEYE_DAISY,
		FLOWER_PAEONIA,
		FLOWER_POT,
		FLOWER_ROSE,
		FLOWER_TULIP_ORANGE,
		FLOWER_TULIP_PINK,
		FLOWER_TULIP_RED,
		FLOWER_TULIP_WHITE,
		FROSTED_ICE_0,
		FROSTED_ICE_1,
		FROSTED_ICE_2,
		FROSTED_ICE_3,
		FURNACE_FRONT_OFF,
		FURNACE_FRONT_ON,
		FURNACE_SIDE,
		FURNACE_TOP,
		GLASS,
		GLASS_BLACK,
		GLASS_BLUE,
		GLASS_BROWN,
		GLASS_CYAN,
		GLASS_GRAY,
		GLASS_GREEN,
		GLASS_LIGHT_BLUE,
		GLASS_LIME,
		GLASS_MAGENTA,
		GLASS_ORANGE,
		GLASS_PANE_TOP,
		GLASS_PANE_TOP_BLACK,
		GLASS_PANE_TOP_BLUE,
		GLASS_PANE_TOP_BROWN,
		GLASS_PANE_TOP_CYAN,
		GLASS_PANE_TOP_GRAY,
		GLASS_PANE_TOP_GREEN,
		GLASS_PANE_TOP_LIGHT_BLUE,
		GLASS_PANE_TOP_LIME,
		GLASS_PANE_TOP_MAGENTA,
		GLASS_PANE_TOP_ORANGE,
		GLASS_PANE_TOP_PINK,
		GLASS_PANE_TOP_PURPLE,
		GLASS_PANE_TOP_RED,
		GLASS_PANE_TOP_SILVER,
		GLASS_PANE_TOP_WHITE,
		GLASS_PANE_TOP_YELLOW,
		GLASS_PINK,
		GLASS_PURPLE,
		GLASS_RED,
		GLASS_SILVER,
		GLASS_WHITE,
		GLASS_YELLOW,
		GLAZED_TERRACOTTA_BLACK,
	    GLAZED_TERRACOTTA_BLUE,
	    GLAZED_TERRACOTTA_BROWN,
	    GLAZED_TERRACOTTA_CYAN,
	    GLAZED_TERRACOTTA_GRAY,
	    GLAZED_TERRACOTTA_GREEN,
	    GLAZED_TERRACOTTA_LIGHT_BLUE,
	    GLAZED_TERRACOTTA_LIME,
	    GLAZED_TERRACOTTA_MAGENTA,
	    GLAZED_TERRACOTTA_ORANGE,
	    GLAZED_TERRACOTTA_PINK,
	    GLAZED_TERRACOTTA_PURPLE,
	    GLAZED_TERRACOTTA_RED,
	    GLAZED_TERRACOTTA_SILVER,
	    GLAZED_TERRACOTTA_WHITE,
	    GLAZED_TERRACOTTA_YELLOW,
		GLOWSTONE,
		GOLD_BLOCK,
		GOLD_ORE,
		GRASS_PATH_SIDE,
		GRASS_PATH_TOP,
		GRASS_SIDE,
		GRASS_SIDE_OVERLAY,
		GRASS_SIDE_SNOWED,
		GRASS_TOP,
		GRAVEL,
		HARDENED_CLAY,
		HARDENED_CLAY_STAINED_BLACK,
		HARDENED_CLAY_STAINED_BLUE,
		HARDENED_CLAY_STAINED_BROWN,
		HARDENED_CLAY_STAINED_CYAN,
		HARDENED_CLAY_STAINED_GRAY,
		HARDENED_CLAY_STAINED_GREEN,
		HARDENED_CLAY_STAINED_LIGHT_BLUE,
		HARDENED_CLAY_STAINED_LIME,
		HARDENED_CLAY_STAINED_MAGENTA,
		HARDENED_CLAY_STAINED_ORANGE,
		HARDENED_CLAY_STAINED_PINK,
		HARDENED_CLAY_STAINED_PURPLE,
		HARDENED_CLAY_STAINED_RED,
		HARDENED_CLAY_STAINED_SILVER,
		HARDENED_CLAY_STAINED_WHITE,
		HARDENED_CLAY_STAINED_YELLOW,
		HAY_BLOCK_SIDE,
		HAY_BLOCK_TOP,
		HOPPER_INSIDE,
		HOPPER_OUTSIDE,
		HOPPER_TOP,
		ICE,
		ICE_PACKED,
		IRON_BARS,
		IRON_BLOCK,
		IRON_ORE,
		IRON_TRAPDOOR,
		ITEMFRAME_BACKGROUND,
		JUKEBOX_SIDE,
		JUKEBOX_TOP,
		LADDER,
		LAPIS_BLOCK,
		LAPIS_ORE,
		LAVA_FLOW,
		LAVA_STILL,
		LEAVES_ACACIA,
		LEAVES_BIG_OAK,
		LEAVES_BIRCH,
		LEAVES_JUNGLE,
		LEAVES_OAK,
		LEAVES_SPRUCE,
		LEVER,
		LOG_ACACIA,
		LOG_ACACIA_TOP,
		LOG_BIG_OAK,
		LOG_BIG_OAK_TOP,
		LOG_BIRCH,
		LOG_BIRCH_TOP,
		LOG_JUNGLE,
		LOG_JUNGLE_TOP,
		LOG_OAK,
		LOG_OAK_TOP,
		LOG_SPRUCE,
		LOG_SPRUCE_TOP,
		MAGMA,
		MELON_SIDE,
		MELON_STEM_CONNECTED,
		MELON_STEM_DISCONNECTED,
		MELON_TOP,
		MOB_SPAWNER,
		MUSHROOM_BLOCK_INSIDE,
		MUSHROOM_BLOCK_SKIN_BROWN,
		MUSHROOM_BLOCK_SKIN_RED,
		MUSHROOM_BLOCK_SKIN_STEM,
		MUSHROOM_BROWN,
		MUSHROOM_RED,
		MYCELIUM_SIDE,
		MYCELIUM_TOP,
		NETHER_BRICK,
		NETHER_WART_BLOCK,
		NETHER_WART_STAGE_0,
		NETHER_WART_STAGE_1,
		NETHER_WART_STAGE_2,
		NETHERRACK,
		NOTEBLOCK,
		/* OBSERVER_BACK,
        OBSERVER_BACK_LIT,
        OBSERVER_FRONT,
        OBSERVER_SIDE,
        OBSERVER_TOP, */
		OBSIDIAN,
		PISTON_BOTTOM,
		PISTON_INNER,
		PISTON_SIDE,
		PISTON_TOP_NORMAL,
		PISTON_TOP_STICKY,
		PLANKS_ACACIA,
		PLANKS_BIG_OAK,
		PLANKS_BIRCH,
		PLANKS_JUNGLE,
		PLANKS_OAK,
		PLANKS_SPRUCE,
		PORTAL,
		POTATOES_STAGE_0,
		POTATOES_STAGE_1,
		POTATOES_STAGE_2,
		POTATOES_STAGE_3,
		PRISMARINE_BRICKS,
		PRISMARINE_DARK,
		PRISMARINE_ROUGH,
		PUMPKIN_FACE_OFF,
		PUMPKIN_FACE_ON,
		PUMPKIN_SIDE,
		PUMPKIN_STEM_CONNECTED,
		PUMPKIN_STEM_DISCONNECTED,
		PUMPKIN_TOP,
		PURPUR_BLOCK,
		PURPUR_PILLAR,
		PURPUR_PILLAR_TOP,
		QUARTZ_BLOCK_BOTTOM,
		QUARTZ_BLOCK_CHISELED,
		QUARTZ_BLOCK_CHISELED_TOP,
		QUARTZ_BLOCK_LINES,
		QUARTZ_BLOCK_LINES_TOP,
		QUARTZ_BLOCK_SIDE,
		QUARTZ_BLOCK_TOP,
		QUARTZ_ORE,
		RAIL_ACTIVATOR,
		RAIL_ACTIVATOR_POWERED,
		RAIL_DETECTOR,
		RAIL_DETECTOR_POWERED,
		RAIL_GOLDEN,
		RAIL_GOLDEN_POWERED,
		RAIL_NORMAL,
		RAIL_NORMAL_TURNED,
		RED_NETHER_BRICK,
		RED_SAND,
		RED_SANDSTONE_BOTTOM,
		RED_SANDSTONE_CARVED,
		RED_SANDSTONE_NORMAL,
		RED_SANDSTONE_SMOOTH,
		RED_SANDSTONE_TOP,
		REDSTONE_BLOCK,
		REDSTONE_DUST_DOT,
		REDSTONE_DUST_LINE0,
		REDSTONE_DUST_LINE1,
		REDSTONE_DUST_OVERLAY,
		REDSTONE_LAMP_OFF,
		REDSTONE_LAMP_ON,
		REDSTONE_ORE,
		REDSTONE_TORCH_OFF,
		REDSTONE_TORCH_ON,
		REEDS,
		REPEATER_OFF,
		REPEATER_ON,
		REPEATING_COMMAND_BLOCK_BACK,
		REPEATING_COMMAND_BLOCK_CONDITIONAL,
		REPEATING_COMMAND_BLOCK_FRONT,
		REPEATING_COMMAND_BLOCK_SIDE,
		SAND,
		SANDSTONE_BOTTOM,
		SANDSTONE_CARVED,
		SANDSTONE_NORMAL,
		SANDSTONE_SMOOTH,
		SANDSTONE_TOP,
		SAPLING_ACACIA,
		SAPLING_BIRCH,
		SAPLING_JUNGLE,
		SAPLING_OAK,
		SAPLING_ROOFED_OAK,
		SAPLING_SPRUCE,
		SEA_LANTERN,
		/* SHULKER_TOP_BLACK,
        SHULKER_TOP_BLUE,
        SHULKER_TOP_BROWN,
        SHULKER_TOP_CYAN,
        SHULKER_TOP_GRAY,
        SHULKER_TOP_GREEN,
        SHULKER_TOP_LIGHT_BLUE,
        SHULKER_TOP_LIME,
        SHULKER_TOP_MAGENTA,
        SHULKER_TOP_ORANGE,
        SHULKER_TOP_PINK,
        SHULKER_TOP_PURPLE,
        SHULKER_TOP_RED,
        SHULKER_TOP_SILVER,
        SHULKER_TOP_WHITE,
        SHULKER_TOP_YELLOW, */
		SLIME,
		SNOW,
		SOUL_SAND,
		SPONGE,
		SPONGE_WET,
		STONE,
		STONE_ANDESITE,
		STONE_ANDESITE_SMOOTH,
		STONE_DIORITE,
		STONE_DIORITE_SMOOTH,
		STONE_GRANITE,
		STONE_GRANITE_SMOOTH,
		STONE_SLAB_SIDE,
		STONE_SLAB_TOP,
		STONEBRICK,
		STONEBRICK_CARVED,
		STONEBRICK_CRACKED,
		STONEBRICK_MOSSY,
		STRUCTURE_BLOCK,
		STRUCTURE_BLOCK_CORNER,
		STRUCTURE_BLOCK_DATA,
		STRUCTURE_BLOCK_LOAD,
		STRUCTURE_BLOCK_SAVE,
		TALLGRASS,
		TNT_BOTTOM,
		TNT_SIDE,
		TNT_TOP,
		TORCH_ON,
		TRAPDOOR,
		TRIP_WIRE,
		TRIP_WIRE_SOURCE,
		VINE,
		WATER_FLOW,
		WATER_OVERLAY,
		WATER_STILL,
		WATERLILY,
		WEB,
		WHEAT_STAGE_0,
		WHEAT_STAGE_1,
		WHEAT_STAGE_2,
		WHEAT_STAGE_3,
		WHEAT_STAGE_4,
		WHEAT_STAGE_5,
		WHEAT_STAGE_6,
		WHEAT_STAGE_7,
		WOOL_COLORED_BLACK,
		WOOL_COLORED_BLUE,
		WOOL_COLORED_BROWN,
		WOOL_COLORED_CYAN,
		WOOL_COLORED_GRAY,
		WOOL_COLORED_GREEN,
		WOOL_COLORED_LIGHT_BLUE,
		WOOL_COLORED_LIME,
		WOOL_COLORED_MAGENTA,
		WOOL_COLORED_ORANGE,
		WOOL_COLORED_PINK,
		WOOL_COLORED_PURPLE,
		WOOL_COLORED_RED,
		WOOL_COLORED_SILVER,
		WOOL_COLORED_WHITE,
		WOOL_COLORED_YELLOW;
	std::vector<TextureImage*> textures;
};

}
}

#endif /* BLOCKTEXTURES_H_ */

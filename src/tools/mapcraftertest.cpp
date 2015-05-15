#include "../mapcraftercore/mc/world.h"
#include "../mapcraftercore/mc/worldcache.h"
#include "../mapcraftercore/renderer/blockimages.h"
#include "../mapcraftercore/renderer/image.h"
#include "../mapcraftercore/renderer/rendermode.h"
#include "../mapcraftercore/renderer/renderview.h"
#include "../mapcraftercore/renderer/tilerenderer.h"
#include "../mapcraftercore/renderer/tileset.h"
#include "../mapcraftercore/renderer/image/dither.h"
#include "../mapcraftercore/renderer/image/palette.h"
#include "../mapcraftercore/renderer/image/quantization.h"
#include "../mapcraftercore/renderer/rendermodes/lighting.h"
#include "../mapcraftercore/util.h"

#include <string>

using namespace mapcrafter;
using namespace mapcrafter::renderer;

/*
static int magic_count = 0;

class Octree;

class Octree {
public:
	Octree(Octree* parent = nullptr)
		: parent(parent), reduced(false), reference(0), red(0), green(0), blue(0) {
		magic_count++;
		//std::cout << "magic: " << magic_count << std::endl;
		for (int i = 0; i < 8; i++)
			children[i] = nullptr;
	};
	
	~Octree() {
		for (int i = 0; i < 8; i++)
			if (children[i])
				delete children[i];
	};

	Octree* getParent() {
		return parent;
	}

	bool hasChildren(int index) {
		return children[index];
	}

	Octree* getChildren(int index) {
		if (index < 0 || index >= 8)
			return nullptr;
		if (children[index]) {
			//std::cout << "found children " << index << std::endl;
			return children[index];
		}
		//std::cout << "create children " << index << std::endl;
		children[index] = new Octree(this);
		return children[index];
	}

	int getChildrenCount() {
		int count = 0;
		for (int i = 0; i < 8; i++)
			if (children[i])
				count++;
		return count;
	}

	bool isLeaf() const {
		for (int i = 0; i < 8; i++)
			if (children[i])
				return false;
		return true;
	}

	bool isReduced() {
		return reduced;
	}

	void reduce() {
		if (reduced)
			return;
		reduced = true;

		for (int i = 0; i < 8; i++) {
			if (!children[i])
				continue;
			reference += children[i]->reference;
			red += children[i]->red;
			green += children[i]->green;
			blue += children[i]->blue;
		}
	}

	int getReference() const {
		return reference;
	}

	int getChildReferences() const {
		assert(!isLeaf());
		int ref = 0;
		for (int i = 0; i < 8; i++)
			if (children[i])
				ref += children[i]->reference;
		return ref;
	}

	RGBAPixel getColor() const {
		assert(reduced || reference > 1);
		return rgba(red / reference, green / reference, blue / reference, 255);
	}

	void setColor(RGBAPixel color) {
		reduced = false;
		reference++;
		red += rgba_red(color);
		green += rgba_green(color);
		blue += rgba_blue(color);
	}

private:
	Octree* parent;
	Octree* children[8];

	bool reduced;
	int reference;
	int red, green, blue;
	
};

int nth_bit(int x, int n) {
	return (x >> n) & 1;
}

struct octree_compare {
	bool operator()(const Octree* octree1, const Octree* octree2) const {
		return octree1 < octree2;
		//int ref1 = octree1->getChildReferences();
		//int ref2 = octree2->getChildReferences();
		//if (octree1 != octree2)
		//	return ref1 < ref2;
		//return false;
	}
};

class OctreePalette : public SimplePalette {
public:
	OctreePalette(const RGBAImage& source, size_t palette_size) : SimplePalette() {
		constructFromImage(source, palette_size);
	}

	virtual ~OctreePalette() {
	}

	Octree* getOctree() {
		return &octree;
	}

private:
	Octree octree;

	void constructFromImage(const RGBAImage& image, size_t palette_size) {
		std::set<Octree*, octree_compare> parents, next_parents;
		for (int x = 0; x < image.getWidth(); x++)
			for (int y = 0; y < image.getHeight(); y++) {
				//std::cout << "adding " << x << " " << y << std::endl;
				Octree* inserted = addColor(image.pixel(x, y));
				//std::cout << inserted->getParent()->getChildReferences() << std::endl;
				next_parents.insert(inserted->getParent());
			}

		size_t leaves_count = 0;
		std::cout << next_parents.size() << std::endl;
		for (int level = 7; level >= 0; level--) {
			parents = next_parents;
			next_parents.clear();

			leaves_count = 0;
			for (auto parent_it = parents.begin(); parent_it != parents.end(); ++parent_it)
				leaves_count += (*parent_it)->getChildrenCount();

			std::cout << "Level " << level << ": " << leaves_count << " leaves" << std::endl;
			while (parents.size() > 0 && leaves_count > palette_size) {
				Octree* parent = *parents.begin();
				//assert(leaf->isLeaf());
				assert(!parent->isLeaf());
				parents.erase(parents.begin());
				next_parents.insert(parent->getParent());
				leaves_count -= parent->getChildrenCount() - 1;
				parent->reduce();
				//std::cout << "Remove parent with " << parent->getChildrenCount() << " children, ";
				//std::cout << "Add new parent with " << parent->getParent()->getChildrenCount() << " children" << std::endl;
				//std::cout << leaves.size() << " -> " << next_leaves.size() << std::endl;
			}
			if (leaves_count <= palette_size)
				break;
		}

		std::cout << "Reduced to " << leaves_count << std::endl;
		for (auto parent_it = parents.begin(); parent_it != parents.end(); ++parent_it)
			for (int i = 0; i < 8; i++)
				if ((*parent_it)->hasChildren(i))
					colors.push_back((*parent_it)->getChildren(i)->getColor());
		for (auto parent_it = next_parents.begin(); parent_it != next_parents.end(); ++parent_it)
			for (int i = 0; i < 8; i++)
				if ((*parent_it)->hasChildren(i)) {
					Octree* children = (*parent_it)->getChildren(i);
					if (!parents.count(children))
						colors.push_back(children->getColor());
				}
		std::cout << "Really? " << colors.size() << std::endl;
	}

	Octree* addColor(RGBAPixel color) {
		uint8_t red = rgba_red(color);
		uint8_t green = rgba_green(color);
		uint8_t blue = rgba_blue(color);

		Octree* node = &octree;
		for (int i = 7; i >= 0; i--) {
			int index = (nth_bit(red, i) << 2) | (nth_bit(green, i) << 1) | nth_bit(blue, i);
			node = node->getChildren(index);
		}

		node->setColor(color);
		return node;
	}
};

int traverseOctree(Octree* octree) {
	if (octree->isLeaf())
		return 1;

	int count = 0;
	for (int i = 0; i < 8; i++) {
		if (octree->hasChildren(i))
			count += traverseOctree(octree->getChildren(i));
	}
	return count;
}
*/

int main(int argc, char** argv) {
	//RGBAImage test(200, 200);
	RGBAImage test;
	test.readPNG("wantyou.png");

	std::set<RGBAPixel> colors;
	for (int x = 0; x < test.getWidth(); x++)
		for (int y = 0; y < test.getHeight(); y++)
			colors.insert(test.pixel(x, y));

	std::cout << "start" << std::endl;
	//OctreePalette palette(test, 256);
	//std::cout << magic_count << " " << traverseOctree(palette.getOctree()) << " " << colors.size() << std::endl;

	//imageDither(test, palette, true);
	//imageColorQuantize(test, 256);
	test.writePNG("test_dithered.png");
	test.writeIndexedPNG("test.png");

	return 0;

	//test.fill(rgba(127, 127, 127, 255), 0, 0, 200, 200);


	// std::vector<RGBAPixel> palette;
	/*
	palette.push_back(rgba(255, 255, 255, 255));
	palette.push_back(rgba(0, 0, 0, 255));
	*/
	/*
	for (int i = 0; i < 256; i += 64)
		palette.push_back(rgba(i, i, i));
	*/

	/*
	palette.push_back(rgba(248, 227, 232));
	palette.push_back(rgba(28, 20, 18));
	palette.push_back(rgba(31, 26, 40));
	palette.push_back(rgba(57, 45, 81));
	palette.push_back(rgba(148, 18, 41));
	palette.push_back(rgba(179, 151, 192));
	palette.push_back(rgba(154, 70, 36));
	palette.push_back(rgba(176, 132, 117));

	test.quantize(TestPalette(palette), true);
	test.writePNG("test_dithered.png");
	return 0;
	*/

	/*
	std::string view_name = "topdown";
	RenderView* view = createRenderView(view_name);

	TextureResources resources;
	resources.setTextureSize(16, 0);
	resources.loadTextures(util::findTextureDir().string());

	BlockImages* images = view->createBlockImages();
	images->loadBlocks(resources);

	mc::World world(argv[1]);
	world.load();
	mc::WorldCache world_cache(world);
	TileSet* tile_set = view->createTileSet(1);
	tile_set->scan(world);

	RenderMode* render_mode = new LightingRenderMode(true, 1.0, false);
	TileRenderer* tile_renderer = view->createTileRenderer(images, 1, &world_cache, render_mode);

	TilePos tiles_min, tiles_max, tiles_offset;

	auto tiles = tile_set->getRequiredRenderTiles();
	for (auto it = tiles.begin(); it != tiles.end(); ++it) {
		if (it == tiles.begin())
			tiles_min = tiles_max = *it;
		tiles_min = TilePos(std::min(tiles_min.getX(), it->getX()), std::min(tiles_min.getY(), it->getY()));
		tiles_max = TilePos(std::max(tiles_max.getX(), it->getX()), std::max(tiles_max.getY(), it->getY()));
	}

	int tile_size = tile_renderer->getTileSize();
	RGBAImage image((tiles_max.getX() - tiles_min.getX() + 1) * tile_size,
			(tiles_max.getY() - tiles_min.getY() + 1) * tile_size);

	for (auto it = tiles.begin(); it != tiles.end(); ++it) {
		LOG(INFO) << *it;
		RGBAImage tile;
		tile_renderer->renderTile(*it, tile);
		TilePos image_tile = *it - tiles_min;
		image.simpleAlphaBlit(tile, image_tile.getX() * tile_size, image_tile.getY() * tile_size);
	}

	image.writePNG(view_name + ".png");
	*/
}

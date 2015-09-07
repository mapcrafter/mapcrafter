import java.lang.System;
import java.util.Random;

class SlimeTest {
	public static boolean isSlimeChunk(long seed, int x, int z) {
		Random random = new Random(seed +
				(long) (x * x * 0x4c1906) +
				(long) (x * 0x5ac0db) + 
				(long) (z * z) * 0x4307a7L +
				(long) (z * 0x5f24f) ^ 0x3ad8025f);
		return random.nextInt(10) == 0;
	}

	public static void main(String[] args) {
		if (args[0].equals("0")) {
			long seed = 42;

			System.out.println("\tstd::set<mc::ChunkPos> slimes = {");
			for (int x = -10; x <= 10; x++) {
				for (int z = -10; z <= 10; z++) {
					if (isSlimeChunk(seed, x, z)) {
						System.out.println("\t\tmc::ChunkPos(" + x + ", " + z + "),");	
					}
				}
			}
			System.out.println("\t};");
		} else if (args[0].equals("1")) {
			long seed = 73;

			int needed = 20;
			int max = 10000;
			Random random = new Random(System.currentTimeMillis());

			System.out.println("\tstd::set<mc::ChunkPos> slimes = {");
			int found = 0;
			while (found < needed) {
				int chunkx = random.nextInt(2*max) - max;
				int chunkz = random.nextInt(2*max) - max;
				if (isSlimeChunk(seed, chunkx, chunkz)) {
					System.out.println("\t\tmc::ChunkPos(" + chunkx + ", " + chunkz + "),");
					found++;
				}
			}
			System.out.println("\t};\n");

			System.out.println("\tstd::set<mc::ChunkPos> not_slimes = {");
			found = 0;
			while (found < needed) {
				int chunkx = random.nextInt(2*max) - max;
				int chunkz = random.nextInt(2*max) - max;
				if (!isSlimeChunk(seed, chunkx, chunkz)) {
					System.out.println("\t\tmc::ChunkPos(" + chunkx + ", " + chunkz + "),");
					found++;
				}
			}
			System.out.println("\t};");
		}
	}
}

#include <jo/jo.h>

#define	LINE_COLOR_TABLE (VDP2_VRAM_A0	+ 0x1f400)
#define	COLOR_RAM_ADR (VDP2_COLRAM	+ 0x00600)
#define TILE_SIZE JO_MULT_BY_2(JO_FIXED_32)
#define TILE_SIZE_HALF JO_FIXED_32
#define FOV 120

/** @brief Cube vertice table */
# define WALL_CUBE_VERTICES(SIZE) \
{ \
  { SIZE, -SIZE, SIZE }, \
  { SIZE, SIZE, SIZE }, \
  { -SIZE, SIZE, SIZE }, \
  { -SIZE, -SIZE, SIZE }, \
  { -SIZE, -SIZE, SIZE }, \
  { -SIZE, SIZE, SIZE }, \
  { -SIZE, SIZE, -SIZE }, \
  { -SIZE, -SIZE, -SIZE }, \
  { -SIZE, SIZE, -SIZE }, \
  { SIZE, SIZE, -SIZE }, \
  { SIZE, -SIZE, -SIZE }, \
  { -SIZE, -SIZE, -SIZE }, \
  { SIZE, SIZE, SIZE }, \
  { SIZE, -SIZE, SIZE }, \
  { SIZE, -SIZE, -SIZE }, \
  { SIZE, SIZE, -SIZE }, \
  { SIZE, -SIZE, SIZE }, \
  { -SIZE, -SIZE, SIZE }, \
  { -SIZE, -SIZE, -SIZE }, \
  { SIZE, -SIZE, -SIZE }, \
  { -SIZE, SIZE, SIZE }, \
  { SIZE, SIZE, SIZE }, \
  { SIZE, SIZE, -SIZE }, \
  { -SIZE, SIZE, -SIZE } \
}

#define	GRTBL(r,g,b)	(((b&0x1f)<<10) | ((g&0x1f)<<5) | (r&0x1f) )
static	Uint16	DepthData[32] = {
	GRTBL( 16, 16, 16 ),
	GRTBL( 16, 16, 16 ),
	GRTBL( 16, 16, 16 ),
	GRTBL( 16, 16, 16 ),
	GRTBL( 16, 16, 16 ),
	GRTBL( 16, 16, 16 ),
	GRTBL( 16, 16, 16 ),
	GRTBL( 16, 16, 16 ),
	GRTBL( 16, 16, 16 ),
	GRTBL( 16, 16, 16 ),
	GRTBL( 16, 16, 16 ),
	GRTBL( 16, 16, 16 ),
	GRTBL( 16, 16, 16 ),
	GRTBL( 16, 16, 16 ),
	GRTBL( 16, 16, 16 ),
	GRTBL( 16, 16, 16 ),
	GRTBL( 15, 15, 15 ),
	GRTBL( 14, 14, 14 ),
	GRTBL( 13, 13, 13 ),
	GRTBL( 12, 12, 12 ),
	GRTBL( 11, 11, 11 ),
	GRTBL( 10, 10, 10 ),
	GRTBL(  9,  9,  9 ),
	GRTBL(  8,  8,  8 ),
	GRTBL(  7,  7,  7 ),
	GRTBL(  6,  6,  6 ),
	GRTBL(  5,  5,  5 ),
	GRTBL(  4,  4,  4 ),
	GRTBL(  3,  3,  3 ),
	GRTBL(  2,  2,  2 ),
	GRTBL(  1,  1,  1 ),
	GRTBL(  0,  0,  0 ),
};

static jo_3d_quad * walls[FOV];
static POINT wall_vertices[] = WALL_CUBE_VERTICES(TILE_SIZE_HALF);
static int wallTextureStart;

// Sounds
static jo_sound win;
static jo_sound kill;
static jo_sound step;
static jo_sound item;

// VDP2 floor image color palette
jo_palette image_pal;

// Enemy
jo_pos3D_fixed enemyPos;
jo_fixed moved = 0;
bool isMoving = false;
int enemyDir = 0;

// Player
// Rotation, position and camera
jo_pos3D rot;
jo_pos3D_fixed pos;
int bobbing = 0;
bool hasKey;
bool escaped;
bool dead;
bool canExit;

#define MAP_WIDTH 33
#define MAP_HEIGHT 33
#define MAP_WALL_STARTID 8
int map[MAP_WIDTH * MAP_HEIGHT] = {
    9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,
    9,0,0,0,9,9,9,0,0,9,0,0,0,0,0,0,9,9,9,9,0,0,0,9,0,0,9,0,0,0,0,0,9,
    9,0,9,0,0,0,9,0,9,9,0,9,9,9,9,0,0,0,9,0,0,9,0,9,0,9,9,0,9,0,9,0,9,
    9,0,9,9,9,0,9,0,9,0,0,9,0,0,0,9,9,0,0,0,9,0,0,9,0,9,0,0,9,0,9,0,9,
    9,0,0,0,9,0,0,0,0,0,9,0,0,9,0,9,9,9,0,9,9,9,0,0,0,0,0,9,9,0,9,9,9,
    9,0,9,0,9,0,9,9,0,9,9,9,9,9,0,9,0,0,0,9,0,0,9,0,9,9,9,9,0,0,0,0,9,
    9,0,9,0,9,0,0,9,9,9,0,9,0,0,0,9,0,9,0,9,9,0,0,0,0,9,9,9,0,9,9,0,9,
    9,0,9,9,9,9,9,9,0,9,0,9,0,9,0,9,9,9,0,0,9,0,9,9,0,0,0,0,0,9,0,0,9,
    9,0,0,0,0,0,0,0,0,9,0,0,0,0,9,9,0,9,0,9,0,0,0,9,9,0,9,0,9,9,0,9,9,
    9,9,0,9,9,9,0,9,0,0,9,9,9,0,0,9,0,9,0,9,0,9,0,9,0,0,9,0,0,0,0,0,9,
    9,9,0,9,9,9,0,9,9,0,0,0,0,9,0,0,0,9,0,9,0,0,9,0,0,9,9,9,9,9,9,0,9,
    9,0,0,9,0,0,0,0,9,9,0,9,0,9,0,9,9,9,9,0,9,0,9,9,0,9,0,9,0,9,0,0,9,
    9,9,9,9,0,9,9,9,9,9,0,9,0,9,0,0,0,0,0,0,9,0,0,0,0,0,0,9,0,9,9,0,9,
    9,0,0,9,0,9,0,0,9,0,9,9,9,9,0,9,0,9,0,9,9,9,0,9,9,9,9,9,0,0,0,0,9,
    9,0,9,0,0,9,9,0,9,0,0,0,0,0,0,9,0,9,9,0,0,0,0,0,0,0,0,9,9,9,9,0,9,
    9,0,9,0,9,9,0,0,9,0,9,9,0,9,9,0,0,0,9,9,0,9,9,9,0,9,0,0,0,0,0,0,9,
    9,0,9,0,0,9,0,9,9,0,0,9,0,0,0,0,0,0,0,0,0,9,0,9,0,9,9,9,0,9,9,0,9,
    9,0,9,9,0,0,0,9,9,9,9,9,0,9,9,0,0,0,9,9,0,9,0,9,0,0,0,9,0,9,0,0,9,
    9,0,0,9,9,0,9,9,0,0,0,0,0,0,9,9,0,9,9,0,0,0,0,9,0,8,0,9,0,9,0,9,9,
    9,9,0,0,0,0,9,9,0,9,0,9,9,9,0,9,0,9,0,9,0,9,9,9,0,0,0,9,0,9,0,9,9,
    9,0,9,0,9,0,0,0,0,9,0,9,0,0,0,0,0,0,0,9,0,0,0,9,9,0,9,0,0,9,0,0,9,
    9,0,0,0,9,0,9,9,0,9,0,0,0,9,9,0,9,9,0,9,9,9,0,0,0,0,0,9,9,9,9,0,9,
    9,9,9,0,0,0,0,9,0,9,9,9,9,9,0,0,0,9,0,0,0,9,9,9,9,9,0,9,0,0,0,0,9,
    9,0,9,9,0,9,0,9,0,0,9,0,0,0,0,9,0,9,9,9,0,9,0,0,0,0,0,0,0,9,9,0,9,
    9,0,0,0,0,9,0,0,9,9,9,0,9,9,9,0,0,9,0,0,0,0,0,0,0,9,9,9,0,0,0,0,9,
    9,9,9,9,0,9,9,9,9,0,0,0,0,9,9,9,9,9,0,9,9,9,0,0,0,9,0,9,0,9,9,0,9,
    9,9,0,9,0,0,0,0,9,0,9,0,9,9,0,9,0,9,0,9,0,0,9,9,9,0,0,9,0,9,0,0,9,
    9,0,0,0,0,9,9,0,0,9,0,0,9,0,0,9,0,0,0,9,0,0,9,0,0,0,9,9,0,9,0,9,9,
    9,9,9,9,9,9,9,0,0,9,0,9,9,0,9,9,9,9,0,9,9,0,9,0,9,9,9,9,0,0,0,9,9,
    9,0,0,0,9,0,0,0,9,0,0,9,0,0,0,9,0,9,0,0,0,0,9,0,9,0,0,9,0,9,0,0,9,
    9,0,1,0,9,0,9,0,9,0,9,9,0,9,0,9,0,9,9,0,9,9,9,0,9,9,0,9,9,9,9,0,9,
    9,0,0,0,0,0,9,0,0,0,9,9,0,9,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,9,
    9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9
};

int mapHitList[MAP_WIDTH * MAP_HEIGHT];

void InitializeGame() {

    // Initialize enemy
    int intSize = jo_fixed2int(TILE_SIZE);
    enemyPos.x = jo_int2fixed(16 * intSize) + TILE_SIZE_HALF;
    enemyPos.y = jo_int2fixed(16 * intSize) + TILE_SIZE_HALF;
    enemyDir = 0;
    moved = 0;
    isMoving = false;

    // Initialize position and rotation
    pos.x = TILE_SIZE + TILE_SIZE_HALF;
	pos.y = TILE_SIZE + TILE_SIZE_HALF;
	pos.z = 0;
    rot.x = 90;
    rot.y = 0;
    rot.z = 180;

    // Initialize status
    hasKey = false;
    escaped = false;
    dead = false;

    // Reload map
    int mapSize = MAP_WIDTH * MAP_HEIGHT;
    for (int mapTile = 0; mapTile < mapSize; mapTile++) {
        map[mapTile] = JO_ABS(map[mapTile]);
    }
}

void UpdateBobbing() {
    if (bobbing != 0) {
        bobbing+=5;

        if (bobbing >= 180) {
            bobbing = 0;
            jo_audio_play_sound_on_channel(&step, 0);
        }
    }
}

int FixAngle(int angle) {
    return angle > 359 ? (angle - 360) : (angle < 0 ? (angle + 360) : angle);
}

int FindIntersectedSide(jo_fixed x, jo_fixed y, jo_fixed rx, jo_fixed ry, jo_fixed rox, jo_fixed roy) {
    int side = -1;
    jo_fixed left = x - TILE_SIZE_HALF;
    jo_fixed right = x + TILE_SIZE_HALF;
    jo_fixed bottom = y - TILE_SIZE_HALF;
    jo_fixed top = y + TILE_SIZE_HALF;

    // Check for collision on horizontal line
    if (rx != 0 &&
        ((rx > 0 && rox <= left && rox + rx > left) ||
         (rx < 0 && rox >= right && rox + rx < right))) {
        // We got collision
        if (rx > 0) {
            side = 3;
        }
        else {
            side = 1;
        }
    }

    // Check for collision on vertical line
    if (ry != 0 &&
        ((ry > 0 && roy <= bottom && roy + ry > bottom) ||
         (ry < 0 && roy >= top && roy + ry < top))) {
        // We got collision
        if (ry > 0) {
            side = 5;
        }
        else {
            side = 4;
        }
    }

    return side;
}

void DrawEntity(int mapX, int mapY) {
    int entityId = map[mapY * MAP_WIDTH + mapX];
    int intSize = jo_fixed2int(TILE_SIZE);
    jo_fixed mapPosX = jo_int2fixed(mapX * intSize) + TILE_SIZE_HALF;
    jo_fixed mapPosY = jo_int2fixed(mapY * intSize) + TILE_SIZE_HALF;

    // Key
    if (entityId == 1) {
        jo_3d_push_matrix();
        {
            jo_3d_rotate_matrix(rot.x, rot.y, rot.z);
            jo_3d_translate_matrix_fixed(pos.x - mapPosX, pos.y - mapPosY, pos.z);

            jo_3d_draw_billboard(0, 0, 0, 0);       
        }
        jo_3d_pop_matrix();
    }
}

void CheckExitDoor(int mapX, int mapY) {

    jo_clear_screen_line(15);
    canExit = false;

    // If door are 1 tile near, player can press A to escape if he has the key
    if (map[mapY * MAP_WIDTH + mapX] == 8 ||
        ((mapX - 1 >= 0 && map[mapY * MAP_WIDTH + (mapX - 1)] == 8) ||
         (mapX + 1 < MAP_WIDTH && map[mapY * MAP_WIDTH + (mapX + 1)] == 8)) ||
        ((mapY - 1 >= 0 && map[(mapY - 1) * MAP_WIDTH + mapX] == 8) ||
         (mapY + 1 < MAP_HEIGHT && map[(mapY + 1) * MAP_WIDTH + mapX] == 8))) {

        // Exit door are near the player
        if (hasKey) {
            jo_printf(8,15,"Press [A] to escape...");
            canExit = true;
        }
        else {
            jo_printf(1,15,"You need the key to open this door...");
        }
    }
}

// Draw map
void DrawMap() {
    static POLYGON polygon[] = { NORMAL(JO_FIXED_0, JO_FIXED_1, JO_FIXED_0), VERTICES(0, 1, 2, 3) };

    int mapSize = MAP_WIDTH * MAP_HEIGHT;
    int intSize = jo_fixed2int(TILE_SIZE);
    int mapPlayerX = jo_fixed2int(pos.x) / intSize;
    int mapPlayerY = jo_fixed2int(pos.y) / intSize;
    //jo_printf(0,1, "%d; %d;; %d                 ", mapPlayerX, mapPlayerY, rot.z);

    // Ray-casting to figure out which walls to draw
    int rayAngle = FixAngle(rot.z + (FOV >> 1));
    int depthOfView = 0;
    int count = 0;
    
    for (int ray = 0; ray < FOV; ray++) {
        depthOfView = 0;
        jo_fixed rayX = pos.x;
        jo_fixed rayY = pos.y;
        jo_fixed rayStepX = -jo_fixed_mult(TILE_SIZE_HALF >> 1, jo_sin(rayAngle));
        jo_fixed rayStepY = -jo_fixed_mult(TILE_SIZE_HALF >> 1, jo_cos(rayAngle));

        // Send ray
        while(depthOfView < 32) {
            int mapX = jo_fixed2int(rayX) / jo_fixed2int(TILE_SIZE);
            int mapY = jo_fixed2int(rayY) / jo_fixed2int(TILE_SIZE);
            int mapOffset = mapY * MAP_WIDTH + mapX;

            if (mapOffset >= 0 && mapOffset < mapSize && map[mapOffset] >= MAP_WALL_STARTID) {
                jo_fixed mapRayX = jo_int2fixed(mapX * intSize) + TILE_SIZE_HALF;
                jo_fixed mapRayY = jo_int2fixed(mapY * intSize) + TILE_SIZE_HALF;
                depthOfView = 32;

                // Find what side to render
                int side = FindIntersectedSide(mapRayX, mapRayY, rayStepX, rayStepY, rayX - rayStepX, rayY - rayStepY) + 1;

                if (side > 0 && mapHitList[mapY * MAP_WIDTH + mapX] != 10 + side) {
                    int dX = (mapPlayerX - mapX);
                    int dY = (mapPlayerY - mapY);
                    int d = jo_sqrt((dX * dX) + (dY * dY));
                    d = JO_MIN(d, 16) / 2;

                    jo_3d_push_matrix();
                    {
                        jo_3d_rotate_matrix(rot.x, rot.y, rot.z);
                        jo_3d_translate_matrix_fixed(pos.x - mapRayX, pos.y - mapRayY, pos.z);

                        // Get texture ID
                        int texture = JO_MAX(d, 0);
                        texture = JO_MIN(texture, 3);

                        // Create quad
                        jo_3d_quad * quad = walls[ray];
                        quad->data.nbPoint = 4;
                        quad->data.pntbl = wall_vertices + ((side - 1) * 4);
                        quad->data.nbPolygon = 1;
                        quad->data.pltbl = polygon;

                        // Set Attribute
                        if (quad->data.attbl == JO_NULL) {
                            quad->data.attbl = jo_malloc(sizeof(ATTR));
                            quad->data.attbl->flag = Dual_Plane;
                            JO_ZERO(quad->data.attbl->gstb);
                            jo_3d_set_texture(quad, 0);
                            JO_ADD_FLAG(quad->data.attbl->sort, UseDepth);
                        }

                        // do not use jo_3d_set_texture, it causes problems
                        quad->data.attbl->texno = wallTextureStart + (map[mapY * MAP_WIDTH + mapX] - MAP_WALL_STARTID);

                        // Draw wall
                        jo_3d_draw(quad);
                    }
                    jo_3d_pop_matrix();

                    // Mark tile as ready for rendered
                    mapHitList[mapY * MAP_WIDTH + mapX] = 10 + side;
                }
            }
            else {
                if (mapOffset >= 0 && mapOffset < mapSize && mapHitList[mapOffset] == 0) {
                    int mapEnemyX = jo_fixed2int(enemyPos.x) / jo_fixed2int(TILE_SIZE);
                    int mapEnemyY = jo_fixed2int(enemyPos.y) / jo_fixed2int(TILE_SIZE);

                    if (map[mapOffset] > 0) {
                        DrawEntity(mapX, mapY);
                        mapHitList[mapOffset] = map[mapOffset];
                    }
                    else if (mapX == mapEnemyX && mapY == mapEnemyY) {
                        // Draw enemy
                        jo_3d_push_matrix();
                        {
                            jo_3d_rotate_matrix(rot.x, rot.y, rot.z);
                            jo_3d_translate_matrix_fixed(pos.x - enemyPos.x, pos.y - enemyPos.y, pos.z);

                            jo_sprite_enable_screen_doors_filter();
                            jo_3d_draw_billboard(1, 0, 0, 0);
                            jo_sprite_disable_screen_doors_filter();   
                        }
                        jo_3d_pop_matrix();
                        mapHitList[mapOffset] = 2;
                    }
                }

                rayX += rayStepX;
                rayY += rayStepY;
                depthOfView++;
            }
        }
        
        rayAngle = FixAngle(rayAngle - 1);
    }

    CheckExitDoor(mapPlayerX, mapPlayerY);

    // Reset hit map
    for (int w = 0; w < MAP_WIDTH; w++) {
        for (int h = 0; h < MAP_HEIGHT; h++) {
            mapHitList[h * MAP_WIDTH + w] = 0;
        }
    }
}

void draw(void)
{
    // Do bobbing
    UpdateBobbing();
    pos.z = -jo_sin(bobbing) << 1;

    // Game end
    if (escaped) {
        jo_clear_screen();
        
        jo_printf(10,15,"You have escaped :)");
        jo_printf(1,16,"Thank you for playing my small game.");
        jo_printf(3,17,"To try play again press [START]");
        return;
    }
    else if (dead){
        jo_clear_screen();
        
        jo_printf(10,15,"You have died :(");
        jo_printf(1,16,"Thank you for playing my small game.");
        jo_printf(3,17,"To try play again press [START]");
        return;
    }

    // Draw the floor
    jo_3d_push_matrix();
	{
        jo_3d_rotate_matrix(rot.x, rot.y, rot.z);
        jo_3d_translate_matrix_fixed(pos.x, pos.y, (pos.z - TILE_SIZE_HALF));
        jo_background_3d_plane_a_draw(true);

	}
	jo_3d_pop_matrix();

    // Draw map
    DrawMap();
    
    // Draw the ceiling
    jo_3d_push_matrix();
	{
        jo_3d_rotate_matrix(rot.x, rot.y, rot.z);
        jo_3d_translate_matrix_fixed(pos.x, pos.y, (pos.z + TILE_SIZE_HALF));
        jo_background_3d_plane_b_draw(true);
	}
	jo_3d_pop_matrix();
}

void PickUpItem() {
    int tileSize = jo_fixed2int(TILE_SIZE);
    int mapX = jo_fixed2int(pos.x) / tileSize;
    int mapY = jo_fixed2int(pos.y) / tileSize;
    int mapOffset = mapY * MAP_WIDTH + mapX;
    bool pickedUpAny = false;

    if (mapOffset >= 0 && mapOffset < MAP_WIDTH * MAP_HEIGHT) {
        if (map[mapOffset] == 1) {
            hasKey = true;
            pickedUpAny = true;
        }
    }

    if (pickedUpAny) {
        map[mapOffset] = -map[mapOffset];
        jo_audio_play_sound_on_channel(&item, 1);
    }
}

void UpdateEnemy() {
    int dx = 0;
    int dy = 0;
    
    switch(enemyDir) {
        case 0:
            dx = -1;
            break;

        case 1:
            dy = -1;
            break;

        case 2:
            dx = 1;
            break;

        default:
            dy = 1;
            break;
    }

    // Kill player if too close
    if (!escaped && !dead) {
        jo_fixed difX = enemyPos.x - pos.x;
        jo_fixed difY = enemyPos.y - pos.y;
        difX = JO_ABS(difX);
        difY = JO_ABS(difY);

        if (difX < TILE_SIZE_HALF && difY < TILE_SIZE_HALF) {
            dead = true;
            jo_audio_play_sound_on_channel(&kill, 3);
        }
    }

    // update movement
    if (isMoving) {
        if (moved >= TILE_SIZE) {
            jo_fixed error = moved - TILE_SIZE;
            enemyPos.x -= dx * error;
            enemyPos.y -= dy * error;
            
            moved = 0;
            isMoving = false;
        }
        else {
            enemyPos.x += dx * JO_FIXED_1_DIV_2;
            enemyPos.y += dy * JO_FIXED_1_DIV_2;
            moved += (JO_ABS(dx) * JO_FIXED_1_DIV_2) + (JO_ABS(dy) * JO_FIXED_1_DIV_2);
        }
    }
    else {
        int tileSize = jo_fixed2int(TILE_SIZE);
        int mapEnemyX = jo_fixed2int(enemyPos.x) / tileSize;
        int mapEnemyY = jo_fixed2int(enemyPos.y) / tileSize;
        int availableDirrections = 0;

        int oldDir = enemyDir + 2;
        if (oldDir > 3) oldDir = oldDir - 4;

        // Check if enemy is not on same square as player
        bool freeDirs[4] = {
            (mapEnemyX - 1 >= 0 && map[mapEnemyY * MAP_WIDTH + (mapEnemyX - 1)] == 0),
            (mapEnemyY - 1 >= 0 && map[(mapEnemyY - 1) * MAP_WIDTH + mapEnemyX] == 0),
            (mapEnemyX + 1 < MAP_WIDTH && map[mapEnemyY * MAP_WIDTH + (mapEnemyX + 1)] == 0),
            (mapEnemyY + 1 < MAP_HEIGHT && map[(mapEnemyY + 1) * MAP_WIDTH + mapEnemyX] == 0)
        };

        for (int dir = 0; dir < 4; dir++) {
            if (dir != oldDir && freeDirs[dir]) availableDirrections++;
        }

        // Check whether direction ghost is going is not a wall, or at crossing
        if (!freeDirs[enemyDir] || availableDirrections > 1) {
            
            // Enemy all other paths are blocked
            if (availableDirrections == 0) {
                enemyDir += 2;
                if (enemyDir > 3) enemyDir = enemyDir - 4;
            }
            else {
                // Change direction to next free block
                int randomDir = -1;

                while (randomDir == -1) {
                    randomDir = jo_random(4) - 1;
                    if (randomDir == oldDir || !freeDirs[randomDir]) randomDir = -1;
                }

                enemyDir = randomDir;
            }
        }

        isMoving = true;
    }
}

void DoCollisions(int movementSpeed) {
    jo_pos2D_fixed oldPos;
    oldPos.x = pos.x;
    oldPos.y = pos.y;
    jo_fixed halfHalfTile = TILE_SIZE_HALF - (TILE_SIZE_HALF >> 2);
    int tileSize = jo_fixed2int(TILE_SIZE);

    // --- HORIZONTAL ---
    pos.x -= jo_fixed_mult(movementSpeed, jo_fixed_div(jo_sin(rot.z), jo_int2fixed(10)));
    int mapPlayerX = jo_fixed2int(pos.x) / tileSize;
    int mapPlayerY = jo_fixed2int(pos.y) / tileSize;

    // Check if outside of world
    if (mapPlayerX < 0 || mapPlayerX > MAP_WIDTH - 1) pos.x = oldPos.x;
    
    // Check for walls
    if (map[mapPlayerY * MAP_WIDTH + mapPlayerX] >= MAP_WALL_STARTID) pos.x = oldPos.x;

    // Keep player certain distance from the wall
    if (mapPlayerX + 1 < MAP_WIDTH && map[mapPlayerY * MAP_WIDTH + (mapPlayerX + 1)] >= MAP_WALL_STARTID &&
        jo_int2fixed((mapPlayerX + 1) * tileSize) - pos.x < halfHalfTile) {
        pos.x = jo_int2fixed((mapPlayerX + 1) * tileSize) - halfHalfTile;
    }
    
    if (mapPlayerX - 1 >= 0 && map[mapPlayerY * MAP_WIDTH + (mapPlayerX - 1)] >= MAP_WALL_STARTID &&
        pos.x - (jo_int2fixed((mapPlayerX - 1) * tileSize) + TILE_SIZE) < halfHalfTile) {
        pos.x = jo_int2fixed((mapPlayerX - 1) * tileSize) + TILE_SIZE + halfHalfTile;
    }

    // --- VERTICAL ---
	pos.y -= jo_fixed_mult(movementSpeed, jo_fixed_div(jo_cos(rot.z), jo_int2fixed(10)));
    mapPlayerX = jo_fixed2int(pos.x) / tileSize;
    mapPlayerY = jo_fixed2int(pos.y) / tileSize;

    // Check if outside of world
    if (mapPlayerY < 0 || mapPlayerY > MAP_HEIGHT - 1) pos.y = oldPos.y;
    
    // Check for walls
    if (map[mapPlayerY * MAP_WIDTH + mapPlayerX] >= MAP_WALL_STARTID) pos.y = oldPos.y;
    
    // Keep player certain distance from the wall
    if (mapPlayerY + 1 < MAP_HEIGHT && map[(mapPlayerY + 1) * MAP_WIDTH + mapPlayerX] >= MAP_WALL_STARTID &&
        jo_int2fixed((mapPlayerY + 1) * tileSize) - pos.y < halfHalfTile) {
        pos.y = jo_int2fixed((mapPlayerY + 1) * tileSize) - halfHalfTile;
    }
    
    if (mapPlayerY - 1 >= 0 && map[(mapPlayerY - 1) * MAP_WIDTH + mapPlayerX] >= MAP_WALL_STARTID &&
        pos.y - (jo_int2fixed((mapPlayerY - 1) * tileSize) + TILE_SIZE) < halfHalfTile) {
        pos.y = jo_int2fixed((mapPlayerY - 1) * tileSize) + TILE_SIZE + halfHalfTile;
    }
}

void input(void)
{
    if (escaped || dead) {
        if (jo_is_pad1_available() && jo_is_pad1_key_down(JO_KEY_START)){
            InitializeGame();
            jo_clear_screen();
        }
    }

    jo_fixed movementSpeed = 0;

    if (jo_is_pad1_available()) {
        if (jo_is_pad1_key_pressed(JO_KEY_LEFT)) {
            rot.z -= 1;
        } 
        else if (jo_is_pad1_key_pressed(JO_KEY_RIGHT)) {
            rot.z += 1;
        }

        if (jo_is_pad1_key_pressed(JO_KEY_UP)) {
            movementSpeed = JO_FIXED_8;
        } 
        else if (jo_is_pad1_key_pressed(JO_KEY_DOWN)) {
            movementSpeed = -JO_FIXED_4;
        }

        if (canExit && jo_is_pad1_key_down(JO_KEY_A)) {
            escaped = true;
            jo_audio_play_sound_on_channel(&win, 4);
        }
    }
    
    if (movementSpeed != 0 && bobbing == 0) {
        bobbing = 1;
    }

    rot.z = FixAngle(rot.z);

    DoCollisions(movementSpeed);
    PickUpItem();
    UpdateEnemy();
}

jo_palette *my_tga_palette_handling(void)
{
    // We create a new palette for each image. It's not optimal but OK for a demo.
    jo_create_palette(&image_pal);
    return (&image_pal);
}

void LoadFloortAndCeiling(void)
{
    jo_core_tv_off();

    jo_enable_background_3d_plane(JO_COLOR_Black);

    // Floor
    jo_img_8bits floorPlane;
    floorPlane.data = JO_NULL;
    jo_tga_8bits_loader(&floorPlane, JO_ROOT_DIR, "FLOOR.TGA", 0);
    jo_background_3d_plane_a_img(&floorPlane, image_pal.id, true, true);
    jo_free_img(&floorPlane);

    // Ceiling
    jo_img_8bits ceilingPlane;
    ceilingPlane.data = JO_NULL;
    jo_tga_8bits_loader(&ceilingPlane, JO_ROOT_DIR, "CEILING.TGA", 0);
    jo_background_3d_plane_b_img(&ceilingPlane, image_pal.id, true, true);
    jo_free_img(&ceilingPlane);

    jo_core_tv_on();
}

void LoadLineColorTable() {
    int line;

    // Sets palette
    Uint16 * Line_Color_Pal0 = (Uint16 *)COLOR_RAM_ADR;
    for(line = 0; line < JO_TV_HEIGHT; line++)
    {
        int color = (line - 128) * 2;
        color = JO_ABS(color);

    	Line_Color_Pal0[line+32] = JO_COLOR_RGB(color,color,color);
    }

    // Set indexes to palette
    Line_Color_Pal0	=(Uint16 *)LINE_COLOR_TABLE;
    for(line = 0; line < JO_TV_HEIGHT; line++)
    {
    	Line_Color_Pal0[line] = line + ((256*3)+32);
    }

    slLineColDisp(RBG0ON);
    slLineColTable((void *)LINE_COLOR_TABLE);
    slColorCalc(CC_RATE | CC_2ND | RBG0ON);
    slColorCalcOn(RBG0ON);
    slColRateLNCL(0x0a);
}

void LoadSounds() {
    jo_audio_load_pcm("STEP.PCM", JoSoundMono16Bit, &step);
    jo_audio_load_pcm("ITEM.PCM", JoSoundMono16Bit, &item);
    jo_audio_load_pcm("DEAD.PCM", JoSoundMono16Bit, &kill);
    jo_audio_load_pcm("WIN.PCM", JoSoundMono16Bit, &win);
}

void jo_main(void) {
    // Initialize engine
    jo_core_init(JO_COLOR_Black);
    *(volatile unsigned char *)0x060FFCD8 = 0x1F;
    slSetDepthLimit(0,8,5);
	slSetDepthTbl(DepthData,0xf000,32);

    // Load textures
    jo_set_tga_palette_handling(my_tga_palette_handling);
    LoadFloortAndCeiling();
    LoadLineColorTable();

    jo_sprite_add_tga(JO_ROOT_DIR, "KEY.TGA", JO_COLOR_Black);
    jo_sprite_add_tga(JO_ROOT_DIR, "GHOST.TGA", JO_COLOR_Green);
    wallTextureStart = jo_sprite_add_tga(JO_ROOT_DIR, "DOOR.TGA", JO_COLOR_Transparent);
    jo_sprite_add_tga(JO_ROOT_DIR, "WALL1.TGA", JO_COLOR_Transparent);

    // Load sounds
    LoadSounds();

    // Initialize walls
    for (int wall = 0; wall < FOV; wall++) {
        walls[wall] = jo_malloc(sizeof(jo_3d_quad));
        walls[wall]->data.attbl = JO_NULL;
    }

    InitializeGame();

    // Start engine
    jo_core_add_callback(input);
    jo_core_add_callback(draw);
    jo_core_run();
}

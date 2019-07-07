// Bullet structure:
//   - coordinate (x, y)
//	 - direction (vx, vy)
//	 - flag which determines whether bullet should be removed
struct bullet {
	float x;
	float y;
	float vx;
	float vy;
	bool remove;
};

// Target structure:
//   - coordinate (x, y)
//	 - flag which determines whether target should be removed
struct target {
	float x;
	float y;
	bool remove;
};

struct object_vec {
	float vx;
	float vy;
	float angle;
};

static bool checkMapBoundary(float x, float y);

static bool checkCollision(float x, float y);

static bool findCollisions(const bullet& b);

static bool moveBullet(bullet& b, float elapsed);

static object_vec getAngleFromPlayer(float x, float y);

static void drawBullet(bullet& b, float* depthBuffer, wchar_t* screen);

static void drawTarget(target& t, float* depthBuffer, wchar_t* screen);

static bool checkWallBoundary(float currX, float currY, float rayX, float rayY);

static short getWallShade(float dist);

static char getFloorShade(float dist);

static void movePlayer(float movX, float movY);
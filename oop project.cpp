#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <SFML/Window.hpp>
#include <ctime> 
#include <cstdlib>
#include <algorithm>

#define spaceShipSpeed 17
#define spaceShipHealth 250 
#define fireDelay 150  // as millisecond

#define enemySpawnDelay 1500
#define enemySpeed 5
#define enmeyFireDelay 1000

#define RewardDelay 15000


using namespace sf;
using namespace std;

bool checkCollision(Sprite s1, Vector2f bulletPos) {
	FloatRect s1Global = s1.getGlobalBounds();
	return s1Global.contains(bulletPos);
}

class Game;

class ResourceLoadFailure {
private:
	string data;
public:
	ResourceLoadFailure(string data) : data(data) {}
	void displayError() { cout << data << endl; }
};

class Bullet {
protected:
	Texture texture;
	Sprite* sprite;
	float speed;
	Music fireSound;


public:
	virtual void move() = 0;
	Sprite* getSprite() { return this->sprite; }

	virtual ~Bullet() {
		delete sprite;
	}

	friend Game;
};

class PlayerBullet : public Bullet {
public:
	PlayerBullet(float x, float y) {
		if (!this->texture.loadFromFile("./resources/images/bullet.png")) throw ResourceLoadFailure("SpaceShip image not found in current directory");
		this->sprite = new Sprite(this->texture);
		this->speed = 20;
		sprite->setPosition(Vector2f{ x,y });
		sprite->setScale(Vector2f{ 0.1f,0.1f });
		if (!fireSound.openFromFile("./resources/Audios/fire sound.wav")) throw ResourceLoadFailure("Fire audio not found in current directory");
	}

	void move() override { sprite->move(Vector2f{ 0,-this->speed }); }
	void playFireSound() { this->fireSound.play(); }

};

class EnemyBullet : public Bullet {
private:
	float power;
public:
	EnemyBullet(float x, float y) {
		if (!this->texture.loadFromFile("./resources/images/EnemyBullet.png")) throw ResourceLoadFailure("Bullet image not found in current directory");
		this->sprite = new Sprite(this->texture);
		this->speed = 15;
		this->power = 20;
		this->sprite->setScale(Vector2f{ 0.3f,0.3f });
		this->sprite->setPosition(Vector2f{ x,y });
	}
	void move() override { this->sprite->move(Vector2f{ 0,this->speed }); }
	float getPower() { return this->power; }
};
class Reward {
protected:
	Texture texture;
	Sprite* sprite;
	Clock Rewarddelay;
	int rewardEnergy;

public:

	Sprite* getSprite() { return this->sprite; }
	virtual ~Reward() { delete this->sprite; }
	int getRewardEnergy() { return this->rewardEnergy; }
};

class HealthReward : public Reward {
public:
	HealthReward(float x, float y) {
		if (!this->texture.loadFromFile("./resources/images/health.png")) throw ResourceLoadFailure("Health reward image not found in current directory");
		this->sprite = new Sprite(this->texture);
		sprite->setPosition(Vector2f{ x,y });
		rewardEnergy = 35;
		sprite->setScale(Vector2f{ 0.1,0.1 });

	}
	void move() { sprite->move(Vector2f{ 0,5 }); }

	friend Game;


};
class SpaceShip {
protected:
	Texture SpaceshipTexture;
	Sprite* SpaceshipSprite;
	float speed;
	Clock fireClock;
	float hieght;
	float widht;
	int health;


public:
	SpaceShip() {
		if (!SpaceshipTexture.loadFromFile("./resources/images/SpaceShip.png")) throw ResourceLoadFailure("Bullet image not found in current directory");
		this->SpaceshipSprite = new Sprite(this->SpaceshipTexture);
		SpaceshipSprite->setScale(Vector2f{ 0.2f,0.2f });
		SpaceshipSprite->setPosition(Vector2f{ 900,900 });
		this->speed = spaceShipSpeed;
		hieght = SpaceshipTexture.getSize().x * SpaceshipSprite->getScale().x;
		widht = SpaceshipTexture.getSize().y * SpaceshipSprite->getScale().y;
		this->health = spaceShipHealth;
	}

	void moveUp() { SpaceshipSprite->move(Vector2f{ 0,-speed }); }
	void moveDown() { SpaceshipSprite->move(Vector2f{ 0,speed }); }
	void moveRight() { SpaceshipSprite->move(Vector2f{ speed,0 }); }
	void moveLeft() { SpaceshipSprite->move(Vector2f{ -speed,0 }); }

	void SpaceShipMovmentOnKeys() {
		Vector2f spaceshipPosition = SpaceshipSprite->getPosition();
		if ((Keyboard::isKeyPressed(Keyboard::Scancode::W) || Keyboard::isKeyPressed(Keyboard::Scancode::Up)) && spaceshipPosition.y >= 0) moveUp();
		if ((Keyboard::isKeyPressed(Keyboard::Scancode::A) || Keyboard::isKeyPressed(Keyboard::Scancode::Left)) && spaceshipPosition.x >= 0) moveLeft();
		if ((Keyboard::isKeyPressed(Keyboard::Scancode::S) || Keyboard::isKeyPressed(Keyboard::Scancode::Down)) && spaceshipPosition.y <= 1080 - widht) moveDown();
		if ((Keyboard::isKeyPressed(Keyboard::Scancode::D) || Keyboard::isKeyPressed(Keyboard::Scancode::Right)) && spaceshipPosition.x <= 1920 - hieght) moveRight();
	}

	void fireBullet(vector<unique_ptr<PlayerBullet>>& b) {
		if (Keyboard::isKeyPressed(Keyboard::Scancode::Space) && fireClock.getElapsedTime().asMilliseconds() >= fireDelay) {
			Vector2f shipPos = SpaceshipSprite->getPosition();
			float bulletX = shipPos.x + widht / 2 - 2;
			float bulletY = shipPos.y;
			auto bullet = make_unique<PlayerBullet>(bulletX, bulletY);
			bullet->playFireSound();
			b.push_back(move(bullet));
			fireClock.restart();
		}
	}

	void gotHit(EnemyBullet& b) {
		this->health -= b.getPower();
	}

	bool isDead() {
		return(this->health <= 0);
	}


	int getHealth() { return this->health; }
	void increaseHealth(int health) {
		if (this->health + health < spaceShipHealth) {
			this->health += health;
		}
		else if (health > 0) {
			this->health = spaceShipHealth;
		}
	}
	~SpaceShip() {
		delete SpaceshipSprite;
	}
	friend Game;
};

class Enemy {
protected:
	Texture texture;
	Sprite* sprite;
	Clock FireClock;
	float Health;

public:
	virtual ~Enemy() { delete sprite; }
	void move() { sprite->move(Vector2f{ 0, enemySpeed }); }
	bool canFire() { return FireClock.getElapsedTime().asMilliseconds() >= enmeyFireDelay; }
	void resetClock() { FireClock.restart(); }
	friend Game;


};

class Enemy1 : public Enemy {
public:
	Enemy1(float x, float y) {
		if (!texture.loadFromFile("./resources/images/Enemy1.png")) throw ResourceLoadFailure("Enemy image not found in current directory");
		sprite = new Sprite(texture);
		sprite->setScale(Vector2f{ 0.2f, 0.2f });
		sprite->setPosition(Vector2f{ x, y });
		Health = 80;
	}

};


class SoundManager {
private:
	Music backgroundmusic;
	Music pointsSound;
	Music hitSound;
	Music introSound;
	Music energyGain;
public:
	SoundManager() {
		if (!backgroundmusic.openFromFile("./resources/Audios/breathing-of-d-epic-action-283161.mp3"))
			throw ResourceLoadFailure("Audio of background not found in current directory");

		if (!pointsSound.openFromFile("./resources/Audios/pointsSound.mp3"))
			throw ResourceLoadFailure("Audio pointsSound.mp3 not found in current directory");

		if (!hitSound.openFromFile("./resources/Audios/gotHit.mp3"))
			throw ResourceLoadFailure("Audio gotHit.mp3 not found in current directory");

		if (!introSound.openFromFile("./resources/Audios/intro.mp3"))
			throw ResourceLoadFailure("Audio intro.mp3 not found in current directory");

		if (!energyGain.openFromFile("./resources/Audios/energyGain.mp3"))
			throw ResourceLoadFailure("Audio energyGain.mp3 not found in current directory");
	}

	void playBackgroundMusic(bool repeat = true) {
		this->backgroundmusic.setLooping(repeat);
		this->backgroundmusic.play();
	}
	void stopBackgroundMusic() { this->backgroundmusic.stop(); }

	void playHitSound() { this->hitSound.play(); }
	void playPointsSound() { this->pointsSound.play(); }
	void playEnegyGainSond() { this->energyGain.play(); }


	void playIntroSound(bool loop = true) {
		this->introSound.setLooping(loop);
		this->introSound.play();
	}
	void stopIntroSound() { this->introSound.stop(); }

	Music& getIntroSound() { return introSound; }


};




class Game {
protected:
	RenderWindow* window;
	Texture backgroundTexture;
	Sprite* backgroundSprite;
	Texture startScreen;
	Sprite* startScreenSprite;

	SoundManager soundManager;

	Font font;
	SpaceShip spaceship;
	vector<unique_ptr<PlayerBullet>> Playerbullets;
	vector<unique_ptr<EnemyBullet>> enemyBullets;
	vector<unique_ptr<Enemy>> enemies;
	vector <unique_ptr<HealthReward>> rewards;
	Clock bulletClock;
	Clock EnemySpawnClock;
	Clock EnemyFireDelay;
	Clock rewardDelay;
	int score;

public:
	Game() {
		VideoMode desktopSize = VideoMode::getDesktopMode();
		window = new RenderWindow(desktopSize, "Space Shooter", Style::Titlebar | Style::Close, State::Fullscreen);
		window->setFramerateLimit(60);
		if (!backgroundTexture.loadFromFile("./resources/images/background.jpg")) throw ResourceLoadFailure("Background image not found in current directory");
		backgroundSprite = new Sprite(backgroundTexture);
		if (!startScreen.loadFromFile("./resources/images/intro.png")) throw ResourceLoadFailure("Start Screen image not found in current directory");
		startScreenSprite = new Sprite(startScreen);

		score = 0;
		if (!font.openFromFile("./resources/fonts/Audiowide-Regular.ttf")) throw ResourceLoadFailure("font not found in current directory");
	}

	void handleEvents() {
		while (optional evnt = window->pollEvent()) {
			if (evnt->is<Event::Closed>()) {
				window->close();
			}
		}
	}

	void update() {
		spaceship.SpaceShipMovmentOnKeys();
		spaceship.fireBullet(Playerbullets);
		spawnEnemies();
		spawnRewards();
		moveBullets();
		moveRewards();
		moveEnemies();
		enemyFiring();
	}

	void moveEnemies() {
		for (int i = enemies.size() - 1; i >= 0; --i) {
			if (enemies[i]) enemies[i]->move();
			if (enemies[i]->sprite->getPosition().y > 1040) {
				enemies.erase(enemies.begin() + i);
			}
		}
	}
	void moveRewards() {
		for (int i = rewards.size() - 1; i >= 0; --i) {
			bool removed = false;
			if (rewards[i]) {
				rewards[i]->move();
				if (checkCollision(*spaceship.SpaceshipSprite, rewards[i]->getSprite()->getPosition())) {
					spaceship.increaseHealth(rewards[i]->getRewardEnergy());
					rewards.erase(rewards.begin() + i);
					soundManager.playEnegyGainSond();
					removed = true;
				}

			}

			if (!removed) {
				if (rewards[i]->sprite->getPosition().y > 1040)  rewards.erase(rewards.begin() + i);
			}


		}
	}


	void enemyFiring() {
		for (auto& EnemY : enemies) {
			if (EnemY->canFire()) {
				Vector2f enemyPos = EnemY->sprite->getPosition();
				float x = EnemY->texture.getSize().x * EnemY->sprite->getScale().x;
				float y = EnemY->texture.getSize().y * EnemY->sprite->getScale().y;
				enemyBullets.push_back(make_unique<EnemyBullet>(enemyPos.x + x / 2, enemyPos.y + y));
				EnemY->resetClock();
			}
		}
	}

	void moveBullets() {
		for (int i = Playerbullets.size() - 1; i >= 0; --i) {
			bool isCollided = false;
			if (Playerbullets[i]) {
				Playerbullets[i]->move();
				for (int j = enemies.size() - 1; j >= 0; --j) {
					if (checkCollision(*enemies[j]->sprite, Playerbullets[i]->getSprite()->getPosition())) {
						enemies.erase(enemies.begin() + j);
						score++;
						soundManager.playPointsSound();
						Playerbullets.erase(Playerbullets.begin() + i);
						isCollided = true;
						break;
					}
				}
			}
			if (!isCollided && Playerbullets[i]->sprite->getPosition().y < -20) {
				Playerbullets.erase(Playerbullets.begin() + i);
			}
		}

		for (int i = enemyBullets.size() - 1; i >= 0; --i) {
			bool isHit = false;
			if (enemyBullets[i]) {
				enemyBullets[i]->move();
				if (checkCollision(*spaceship.SpaceshipSprite, enemyBullets[i]->getSprite()->getPosition())) {
					spaceship.gotHit(*enemyBullets[i]);
					soundManager.playHitSound();
					isHit = true;
					enemyBullets.erase(enemyBullets.begin() + i);
					if (spaceship.isDead()) {
						endScreen();
						window->close();
					}


				}
			}
			if (!isHit) {
				if (enemyBullets[i]->sprite->getPosition().y > 1040) {
					enemyBullets.erase(enemyBullets.begin() + i);
				}
			}
		}
	}

	void spawnEnemies() {
		if (EnemySpawnClock.getElapsedTime().asMilliseconds() >= enemySpawnDelay) {
			enemies.push_back(make_unique<Enemy1>(rand() % 1800, -50));
			EnemySpawnClock.restart();
		}
	}

	void spawnRewards() {
		if (rewardDelay.getElapsedTime().asMilliseconds() >= RewardDelay) {
			rewards.push_back(make_unique<HealthReward>(rand() % 1800, -50));
			rewardDelay.restart();

		}
	}

	void render() {
		window->clear();
		window->draw(*backgroundSprite);
		window->draw(*spaceship.SpaceshipSprite);
		Text t1(font);
		t1.setString(ScoreString(score));

		string Health = "Health : " + to_string(spaceship.getHealth());
		Text t2(font);
		t2.setString(Health);
		t2.setPosition(Vector2f{ 1700,0 });

		window->draw(t1);
		window->draw(t2);

		for (auto& reward : rewards) window->draw(*reward->sprite);
		for (auto& eneMy : enemies) window->draw(*eneMy->sprite);
		for (auto& bulleTs : enemyBullets) window->draw(*bulleTs->sprite);
		for (auto& bulleTs : Playerbullets) window->draw(*bulleTs->sprite);
		window->display();
	}

	string ScoreString(int score) {
		return "Score : " + to_string(score);
	}

	void endScreen() {
		soundManager.stopBackgroundMusic();
		soundManager.playIntroSound();

		RectangleShape fadeOverlay(Vector2f(1920, 1080));
		fadeOverlay.setFillColor(Color(0, 0, 0, 255));


		Text gameOverText(font, "GAME OVER");
		gameOverText.setCharacterSize(100);
		gameOverText.setFillColor(Color::Red);
		gameOverText.setStyle(Text::Bold);
		gameOverText.setPosition(Vector2f(600, 200));

		string finalScore = "Your Score: " + to_string(score);
		Text scoreText(font, finalScore);
		scoreText.setCharacterSize(60);
		scoreText.setFillColor(Color::White);
		scoreText.setPosition(Vector2f(700, 350));

		Text continueText(font, "Press ESC to Exit");
		continueText.setCharacterSize(40);
		continueText.setFillColor(Color(180, 180, 180));
		continueText.setPosition(Vector2f(750, 550));

		while (window->isOpen()) {
			handleEvents();
			if (Keyboard::isKeyPressed(Keyboard::Scancode::Escape)) {
				window->close();
			}

			window->clear();
			window->draw(fadeOverlay);
			window->draw(gameOverText);
			window->draw(scoreText);
			window->draw(continueText);
			window->display();
		}
	}
	void StartScreen() {
		soundManager.playIntroSound();
		bool isKeypressed = false;
		RectangleShape fadeOverlay(Vector2f(1920, 1080));
		fadeOverlay.setFillColor(Color(0, 0, 0, 0));

		while (window->isOpen() && !(Keyboard::isKeyPressed(Keyboard::Scancode::Escape))) {
			while (optional evnt = window->pollEvent()) {
				if (evnt->is<Event::KeyPressed>()) {
					isKeypressed = true;
				}
			}

			window->clear();
			window->draw(*startScreenSprite);

			if (isKeypressed) {

				for (int alpha = 0; alpha <= 255; alpha += 5) {
					fadeOverlay.setFillColor(Color(0, 0, 0, alpha));
					window->draw(fadeOverlay);
					window->display();
					float volume = max(0.f, 100.f - (alpha / 255.f) * 100.f);
					soundManager.getIntroSound().setVolume(volume);
					sleep(milliseconds(25));
				}

				soundManager.stopIntroSound();
				window->clear();
				break;
			}

			window->display();
		}
	}


	void start() {
		StartScreen();
		soundManager.playBackgroundMusic();
		window->setMouseCursorVisible(false);
		while (window->isOpen() && !Keyboard::isKeyPressed(Keyboard::Scancode::Escape)) {
			handleEvents();
			update();
			render();
		}

	}

	~Game() {
		delete window;
		delete backgroundSprite;
		delete startScreenSprite;
	}
};

int main() {
	try {
		srand(time(NULL));
		Game game;
		game.start();
		
	}
	catch (ResourceLoadFailure& e) {
		e.displayError();
		return 1;
	}

	return 0;
}
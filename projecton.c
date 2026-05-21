#include "windows.h"
#include <advanlys.h>
#include <cvirte.h>
#include <ansi_c.h>
#include <userint.h>
#include <utility.h>
#include "bass.h"
#include "projecton.h"
#include <rs232.h>
#include <time.h>

typedef struct
{
	CmtThreadFunctionID threadid;
	int status;
} thread_data;

typedef struct
{
	int x, y;
	int type;  // Type of enemy (0: big, 1: medium, 2: small)
	int health; 
	int velocity; 
	int active; 
} Enemy;

typedef struct
{
	int x, y;
	int health;
	int score;
} Hero;

typedef struct
{
	int x, y;
	int active; 
} Projectile;

static int mainpanelHandle, diffpanelHandle, statisticspanelHandle, gamepanelHandle, menuebarHandle, pausepanelHandle, GOpanelHandle, statspanelHandle;
int player_bitmapid, smallenemy_bitmapid, medenemy_bitmapid, bigenemy_bitmapid, heart_bitmapid, background_bitmapid, hero_laser, enemy_laser;
int lives, difficulty_factor, curser_coordinates, fire_butt = 0, pause = 0, unpause = 0, sound_flag = 1;
int comPort = 4, baudRate = 9600, status, but_pin, x_val, cont, graph;
char scoreText[20];
CmtThreadLockHandle lockhandle;
thread_data *player_thread;				
thread_data *enemy_thread;
thread_data *damage_thread;
Hero player;
Enemy enemies[5];
Projectile enemyProjectiles[5];
Projectile playerProjectiles[7];
HSTREAM player_fire_sound, gameover_sound, enemy_explosion_sound, player_damaged, background_theme_music;
FILE *file;


void SavePlayerStatsToCSV(int score, int difficulty);
int CVICALLBACK drawplayer_treadfunc (void *functionData);
int CVICALLBACK drawenemy_threadfunc(void *functionData);
int CVICALLBACK damage_threadfunc(void *functionData);

int isKeyPressed(int virtualKey)
{
	return GetAsyncKeyState(virtualKey) & 0x8000;
}

int main (int argc, char *argv[])
{
	if (InitCVIRTE (0, argv, 0) == 0)
		return -1;	/* out of memory */
	if ((mainpanelHandle = LoadPanel (0, "projecton.uir", PANEL_1)) < 0)
		return -1;
	if ((diffpanelHandle = LoadPanel (0, "projecton.uir", PANEL_2)) < 0)
		return -1;
	if ((statisticspanelHandle = LoadPanel (0, "projecton.uir", PANEL_3)) < 0)
		return -1;
	if ((gamepanelHandle = LoadPanel (0, "projecton.uir", PANEL_4)) < 0)
		return -1;
	if ((pausepanelHandle = LoadPanel (0, "projecton.uir", PANEL_5)) < 0)
		return -1;
	if ((GOpanelHandle = LoadPanel (0, "projecton.uir", PANEL_6)) < 0)
		return -1;
	if ((statspanelHandle = LoadPanel (0, "projecton.uir", PANEL_7)) < 0)
		return -1;
	player_thread = calloc(1, sizeof(thread_data));	
	enemy_thread =  calloc(1, sizeof(thread_data));
	damage_thread =  calloc(1, sizeof(thread_data));
	CmtNewLock (NULL, OPT_TL_SUPPORT_TIMEOUT, &lockhandle);	//initilaize lock
	menuebarHandle = GetPanelMenuBar (mainpanelHandle);
	DisplayPanel (mainpanelHandle);
	RunUserInterface ();
	return 0;
}


int CVICALLBACK mainpanelfunc (int panel, int event, void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_GOT_FOCUS:
			//load bitmapids for later use
			GetBitmapFromFileEx ("sci_fi_pics\\space_ship.png", 0, &player_bitmapid);
			GetBitmapFromFileEx("sci_fi_pics\\small_ailen.png", 0, &smallenemy_bitmapid);
			GetBitmapFromFileEx("sci_fi_pics\\medium_ailen.png", 0,&medenemy_bitmapid);
			GetBitmapFromFileEx ("sci_fi_pics\\ailen_big.png", 0, &bigenemy_bitmapid);
			GetBitmapFromFileEx ("sci_fi_pics\\heart.png", 0, &heart_bitmapid);
			GetBitmapFromFileEx ("sci_fi_pics\\spaceeee.png", 0, &background_bitmapid);
			GetBitmapFromFileEx ("sci_fi_pics\\laser2.png", 0, &hero_laser);
			GetBitmapFromFileEx ("sci_fi_pics\\enemy_laser2.png", 0, &enemy_laser);
			//initilize bass library for sound effects
			BASS_Init(-1, 44100, 0, 0, NULL);
			//define different sounds
			player_fire_sound = BASS_StreamCreateFile(FALSE, "sounds\\player_shoot.mp3", 0, 0, 0);
			enemy_explosion_sound = BASS_StreamCreateFile(FALSE, "sounds\\explosion.mp3", 0, 0, 0);
			player_damaged = BASS_StreamCreateFile(FALSE, "sounds\\hero_damaged.mp3", 0, 0, 0);
			background_theme_music = BASS_StreamCreateFile(FALSE, "sounds\\background_music.mp3", 0, 0, BASS_SAMPLE_LOOP);
			gameover_sound = BASS_StreamCreateFile(FALSE, "sounds\\gameover.mp3", 0, 0, 0);
			//start palying theme music when first opening the game
			if (sound_flag)
			{
				BASS_ChannelSetAttribute(background_theme_music, BASS_ATTRIB_VOL, 0.3);
				BASS_ChannelPlay(background_theme_music, FALSE);
			}
			break;
		case EVENT_LOST_FOCUS:

			break;
		case EVENT_CLOSE:
			//discard different channels
			DiscardPanel (mainpanelHandle);
			DiscardPanel (diffpanelHandle);
			DiscardPanel (statisticspanelHandle);
			DiscardPanel (gamepanelHandle);
			DiscardPanel (pausepanelHandle);
			DiscardPanel (GOpanelHandle);
			DiscardPanel (statspanelHandle);
			//free threads and lock
			free(player_thread);
			free(enemy_thread);
			free(damage_thread);
			CmtDiscardLock(lockhandle);
			// Free sound stream
			BASS_StreamFree(player_fire_sound);
			BASS_StreamFree(enemy_explosion_sound);
			BASS_StreamFree(player_damaged);
			BASS_StreamFree(background_theme_music);
			BASS_StreamFree(gameover_sound);
			// Free BASS resources
			BASS_Free();
			//close comm port
			CloseCom (4);
			break;
	}
	return 0;
}

int CVICALLBACK exitbutfunc (int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			QuitUserInterface(0);
			break;
	}
	return 0;
}

// when pressing "new game" the choose difficulty window opens.
int CVICALLBACK newgamebutfunc (int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			HidePanel (mainpanelHandle);
			DisplayPanel (diffpanelHandle);
			break;
	}
	return 0;
}

//when pressing "return" the main window will open.
int CVICALLBACK rtrndiffmainfunc (int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			HidePanel (diffpanelHandle);
			DisplayPanel (mainpanelHandle);
			break;
	}
	return 0;
}

//when pressing "game statistics" the statistics window will open.
int CVICALLBACK statbutfunc (int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			HidePanel (mainpanelHandle);
			DisplayPanel (statisticspanelHandle);
			break;
	}
	return 0;
}

//when pressing "return" the main window will open.
int CVICALLBACK rtrnstatmainfunc (int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			HidePanel (statisticspanelHandle);
			DisplayPanel (mainpanelHandle);
			break;
	}
	return 0;
}

//uppon pressing start the function resets the player main traits, and sets the threads to work
int CVICALLBACK startbutfunc (int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
	static int x = 0;
	switch (event)
	{
		case EVENT_COMMIT:

			//gets the player chosen difficulty level
			GetCtrlVal (diffpanelHandle, PANEL_2_DIFFRING, &difficulty_factor);
			//gets the chosen controller
			GetCtrlVal (diffpanelHandle, PANEL_2_CONTROLLER, &cont);
			if (cont)
				OpenComConfig (4, "", 9600, 0, 8, 1, 256, 256);

			HidePanel(diffpanelHandle);
			DisplayPanel(gamepanelHandle);
			player.health = difficulty_factor;
			player.score = 0;
			player.x = 500;
			x = !x;
			//set thread status
			player_thread->status = x;
			enemy_thread->status = x;
			damage_thread->status = x;
			if(x)
			{
				//Notifies a thread pool that you want to execute a function in a thread from the pool
				CmtScheduleThreadPoolFunction (DEFAULT_THREAD_POOL_HANDLE, drawplayer_treadfunc, player_thread, &player_thread->threadid);
				CmtScheduleThreadPoolFunction (DEFAULT_THREAD_POOL_HANDLE, drawenemy_threadfunc, enemy_thread, &enemy_thread->threadid);
				CmtScheduleThreadPoolFunction (DEFAULT_THREAD_POOL_HANDLE, damage_threadfunc, damage_thread, &damage_thread->threadid);
			}
			else
			{
				CmtWaitForThreadPoolFunctionCompletion (DEFAULT_THREAD_POOL_HANDLE, player_thread->threadid, OPT_TP_PROCESS_EVENTS_WHILE_WAITING);
				CmtWaitForThreadPoolFunctionCompletion (DEFAULT_THREAD_POOL_HANDLE, enemy_thread->threadid, OPT_TP_PROCESS_EVENTS_WHILE_WAITING);
				CmtWaitForThreadPoolFunctionCompletion (DEFAULT_THREAD_POOL_HANDLE, damage_thread->threadid, OPT_TP_PROCESS_EVENTS_WHILE_WAITING);
				CmtReleaseThreadPoolFunctionID (DEFAULT_THREAD_POOL_HANDLE, player_thread->threadid);
				CmtReleaseThreadPoolFunctionID (DEFAULT_THREAD_POOL_HANDLE, enemy_thread->threadid);
				CmtReleaseThreadPoolFunctionID (DEFAULT_THREAD_POOL_HANDLE, damage_thread->threadid);
			}
			break;
	}
	return 0;
}


//thread function that handels the damage taken or not from projectiles or collision
int CVICALLBACK damage_threadfunc(void *functionData)
{
	int success;
	while (damage_thread->status)
	{
		CmtGetLockEx(lockhandle, 0, CMT_WAIT_FOREVER, &success);
		if (success)
		{
			//pause game mechanism
			if (isKeyPressed(VK_ESCAPE))
			{
				pause = 1;
				DisplayPanel(pausepanelHandle);
			}
			while (pause)
			{
				if (success)
				{
					CmtReleaseLock(lockhandle); //Release lock while game is paused
					success = 0;
				}
				ProcessSystemEvents();

				if(unpause)
					CmtGetLockEx(lockhandle, 0, CMT_WAIT_FOREVER, &success); //Reacuire lock
			}
			if (success)
			{
				//Ensures that the backgroung keeps on playing unless muted
				if ((BASS_ChannelIsActive(background_theme_music) == BASS_ACTIVE_STOPPED) && sound_flag)
					BASS_ChannelPlay(background_theme_music, TRUE);

				/////////////////////////////////////////////////////////////////////////////////
				//                   track for player - enemy collision                        //
				// this sagment loops over the active enemies                                  //
				// if the coordinates of the an enemy overlap with the players its a collision //
				// uppon a collision the enemy will be deactivated the player will			   //
				// loose ont life and gain score equivalent to the enemy type 				   //
				/////////////////////////////////////////////////////////////////////////////////
				for (int i = 0; i < 5; i++)
					if(enemies[i].active)
						switch (enemies[i].type)
						{
							case 0:		//Big enemy
								if ((player.x + 50) > enemies[i].x && player.x < (enemies[i].x + 150) && 650 > enemies[i].y && 600 < (enemies[i].y + 150))
								{
									if(sound_flag)
									{
										BASS_ChannelSetAttribute(enemy_explosion_sound, BASS_ATTRIB_VOL, 0.08);
										BASS_ChannelPlay(enemy_explosion_sound, TRUE);
									}
									player.score += 15;
									player.health --;
									enemies[i].active = 0;
								}
								break;
							case 1:		//Medium enemy
								if ((player.x + 50) > enemies[i].x && player.x < (enemies[i].x + 76) && 650 > enemies[i].y && 600 < (enemies[i].y + 76))
								{
									if(sound_flag)
									{
										BASS_ChannelSetAttribute(enemy_explosion_sound, BASS_ATTRIB_VOL, 0.08);
										BASS_ChannelPlay(enemy_explosion_sound, TRUE);
									}
									player.score += 10;
									player.health --;
									enemies[i].active = 0;
								}
								break;
							case 2:		//Small enemy
								if ((player.x + 50) > enemies[i].x && player.x < (enemies[i].x + 50) && 650 > enemies[i].y && 600 < (enemies[i].y + 50))
								{
									if(sound_flag)
									{
										BASS_ChannelSetAttribute(enemy_explosion_sound, BASS_ATTRIB_VOL, 0.08);
										BASS_ChannelPlay(enemy_explosion_sound, TRUE);
									}
									player.score += 5;
									player.health --;
									enemies[i].active = 0;
								}
								break;
						}

				////////////////////////////////////////////////////////////////////////////////
				//                 track player projectiles and detect hits                   //
				// this sagment loops over the active enemies and players projectiles         //
				// if the coordinates of an enemy overlap with one of the players projectiles //
				// its a hit, a small explotion sound will play, and the enemy's health 	  //
				// will decrese by 1. if enemy's health is <= 0, the enmy will be deactivated //
				// dissapear and the player will gain respective score						  //
				////////////////////////////////////////////////////////////////////////////////
				int size = sizeof(playerProjectiles) / sizeof(playerProjectiles[0]);
				for (int j = 0; j < size ; j++)
					for (int k = 0; k < 5; k++)
						if(enemies[k].active)
							switch (enemies[k].type)
							{
								case 0:		//Big enemy
									if ((playerProjectiles[j].x + 16) > enemies[k].x && playerProjectiles[j].x < (enemies[k].x + 150) && (playerProjectiles[j].y + 16) > enemies[k].y && playerProjectiles[j].y < (enemies[k].y + 150))
									{
										playerProjectiles[j].active = 0;
										enemies[k].health --;
										if (enemies[k].health <= 0)
										{
											if(sound_flag)
											{
												BASS_ChannelSetAttribute(enemy_explosion_sound, BASS_ATTRIB_VOL, 0.08);
												BASS_ChannelPlay(enemy_explosion_sound, TRUE);
											}
											enemies[k].active = 0;
											player.score += 15;
										}
									}
									break;
								case 1:		 //Medium enemy
									if ((playerProjectiles[j].x + 16) > enemies[k].x && playerProjectiles[j].x < (enemies[k].x + 76) && (playerProjectiles[j].y + 16) > enemies[k].y && playerProjectiles[j].y < (enemies[k].y + 76))
									{
										playerProjectiles[j].active = 0;
										enemies[k].health --;
										if (enemies[k].health <= 0)
										{
											if(sound_flag)
											{
												BASS_ChannelSetAttribute(enemy_explosion_sound, BASS_ATTRIB_VOL, 0.08);
												BASS_ChannelPlay(enemy_explosion_sound, TRUE);
											}
											enemies[k].active = 0;
											player.score += 10;
										}
									}
									break;
								case 2:		//Small enemy
									if ((playerProjectiles[j].x + 16) > enemies[k].x && playerProjectiles[j].x < (enemies[k].x + 50) && (playerProjectiles[j].y + 20) > enemies[k].y && playerProjectiles[j].y < (enemies[k].y + 50))
									{
										playerProjectiles[j].active = 0;
										enemies[k].health --;
										if (enemies[k].health <= 0)
										{
											if(sound_flag)
											{
												BASS_ChannelSetAttribute(enemy_explosion_sound, BASS_ATTRIB_VOL, 0.08);
												BASS_ChannelPlay(enemy_explosion_sound, TRUE);
											}
											enemies[k].active = 0;
											player.score += 5;
										}
									}
									break;
							}

				////////////////////////////////////////////////////////////////////////////////
				//                 track enemy projectiles and detect hits                    //
				// this sagment loops over the enemy projectiles and tracks thier coordinates //
				// if the coordinates overlap with the player's coordinates then its a hit	  //
				// a small explotion sound will play, and the player's health will decrese	  //
				// by 1. if player's health is <= 0, then its a GameOver.					  //
				////////////////////////////////////////////////////////////////////////////////
				int size2 = sizeof(enemyProjectiles) / sizeof(enemyProjectiles[0]);
				for (int l = 0; l < size2 ; l++)
					if (enemyProjectiles[l].active)
					{
						if (enemyProjectiles[l].x <= (player.x + 50) && enemyProjectiles[l].x >= player.x && enemyProjectiles[l].y >= 600 && enemyProjectiles[l].y <= 650)
						{
							if(sound_flag)
							{
								BASS_ChannelSetAttribute(player_damaged, BASS_ATTRIB_VOL, 0.1);
								BASS_ChannelPlay(player_damaged, TRUE);
							}
							enemyProjectiles[l].active = 0;
							player.health --;
						}
					}
			}
		}
		CmtReleaseLock(lockhandle);
		Sleep(10);
	}
	return 0;
}

//thread function that handels enemy shipes spawning and shooting
int CVICALLBACK drawenemy_threadfunc(void *functionData)
{
	srand (time(NULL));
	int success;
	while (enemy_thread->status)
	{
		CmtGetLockEx(lockhandle, 0, CMT_WAIT_FOREVER, &success);
		if (success)
		{
			//pause game mechanism
			if (isKeyPressed(VK_ESCAPE))
			{
				pause = 1;
				DisplayPanel(pausepanelHandle);
			}
			while (pause)
			{
				if (success)
				{
					CmtReleaseLock(lockhandle); //Release lock while game is paused
					success = 0;
				}
				ProcessSystemEvents();

				if(unpause)
					CmtGetLockEx(lockhandle, 0, CMT_WAIT_FOREVER, &success); //Reacuire lock
			}
			if(success)
			{
				CanvasStartBatchDraw (gamepanelHandle, PANEL_4_CANVAS);
				///////////////////////////////////////////////////////////////////////
				// new enemy will spawn at a random x coordinate and initial y is 0 //
				// velocity: big enemy = 1 | medium enemy = 2 | small enemy = 3		 //
				// helth: big enemy = 3    | mediun enemy = 2 | small enemy = 1		 //
				///////////////////////////////////////////////////////////////////////
				for (int i = 0; i < 5; i++)
				{
					//if enemy in 'i' index doesnt exist (inactive) then create one
					if (!enemies[i].active)
					{
						enemies[i].y = 0;
						enemies[i].type = rand() % 3;
						switch (enemies[i].type)
						{
							case 0:
								enemies[i].x = rand() % 850;
								// Check for overlap with other active enemies
								for (int j = 0; j < i; j++)
									if (enemies[j].active)
										if (enemies[j].x >= enemies[i].x && enemies[j].x <= (enemies[i].x + 150))
										{
											i--;  //place the enemy again
											break;
										}
								// If there is no overlap, activate the enemy
								enemies[i].active = 1;
								enemies[i].health = 3;
								enemies[i].velocity = 1;
								break;
							case 1:
								enemies[i].x = rand() % 924;
								// Check for overlap with other active enemies
								for (int k = 0; k < i; k++)
									if (enemies[k].active)
										if (enemies[k].x >= enemies[i].x && enemies[k].x <= (enemies[i].x + 76))
										{
											i--;  //place the enemy again
											break;
										}
								// If there is no overlap, activate the enemy
								enemies[i].active = 1;
								enemies[i].health = 2;
								enemies[i].velocity = 2;
								break;
							case 2:
								enemies[i].x = rand() % 950;
								// Check for overlap with other active enemies
								for (int l = 0; l < i; l++)
									if (enemies[l].active)
										if (enemies[l].x >= enemies[i].x && enemies[l].x <= (enemies[i].x + 50))
										{
											i--;  //place the enemy again
											break;
										}
								// If there is no overlap, activate the enemy
								enemies[i].active = 1;
								enemies[i].health = 1;
								enemies[i].velocity = 3;
								break;
						}
					}
					//if enemy does exist than move it down
					else
					{
						enemies[i].y += enemies[i].velocity;	// Move enemy downwards
						if (enemies[i].y > 700)					// If enemy reaches the bottom, deactivate it
							enemies[i].active = 0;
					}
					// Draw the enemy
					if (enemies[i].active)
						if (enemies[i].type != 0)	// if the type is not 0 or 1 it must be 2 therefor small enemy
							if(enemies[i].type != 1)
								CanvasDrawBitmap(gamepanelHandle, PANEL_4_CANVAS, smallenemy_bitmapid, VAL_ENTIRE_OBJECT,MakeRect(enemies[i].y, enemies[i].x, 50, 50));
							else
								CanvasDrawBitmap(gamepanelHandle, PANEL_4_CANVAS, medenemy_bitmapid, VAL_ENTIRE_OBJECT,MakeRect(enemies[i].y, enemies[i].x, 76, 76));
						else
							CanvasDrawBitmap(gamepanelHandle, PANEL_4_CANVAS, bigenemy_bitmapid, VAL_ENTIRE_OBJECT,MakeRect(enemies[i].y, enemies[i].x, 150, 150) );
				}

				///////////////////////////////////////////////////////////////////////////////
				//		                 enemy shooting handling							 //
				// each enemy will shoot a new projectile after 2 seconds					 //
				// all projectiles are generated at the center of the enemy and act the same //
				///////////////////////////////////////////////////////////////////////////////
				static time_t lastShotTimes[5] = {0}; // Track last shot time for each enemy
				for (int i = 0; i < 5; i++)
				{
					if (enemies[i].active)
					{
						time_t currentTime = time(NULL);
						if (difftime(currentTime, lastShotTimes[i]) > 2.0)  // Check if more than 2 seconds have passed
						{
							// Spawn projectile for the enemy
							for (int j = 0; j < 5; j++)
								if (!enemyProjectiles[j].active)
								{
									// set correct enemy center coordinates
									switch (enemies[i].type)
									{
										case 0:
											enemyProjectiles[j].x = enemies[i].x + 65;
											enemyProjectiles[j].y = enemies[i].y + 150;
											break;
										case 1:
											enemyProjectiles[j].x = enemies[i].x + 28;
											enemyProjectiles[j].y = enemies[i].y + 76;
											break;
										case 2:
											enemyProjectiles[j].x = enemies[i].x + 15;
											enemyProjectiles[j].y = enemies[i].y + 50;
											break;
									}
									enemyProjectiles[j].active = 1;
									lastShotTimes[i] = currentTime; // Update last shot time
									break;
								}
						}
						// Update all active enemy projectiles
						for (int k = 0; k < 5; k++)
							if (enemyProjectiles[k].active)
							{
								enemyProjectiles[k].y += 1;		// Move projectile downward
								// Draw projectile
								Rect projectileRect = MakeRect(enemyProjectiles[k].y, enemyProjectiles[k].x, 20, 20);
								CanvasDrawBitmap(gamepanelHandle, PANEL_4_CANVAS, enemy_laser, VAL_ENTIRE_OBJECT, projectileRect);
								// If projectile goes off-screen, deactivate it
								if (enemyProjectiles[k].y > 700)
									enemyProjectiles[k].active = 0;
							}
					}
				}
				// Update the entire canvas to reflect changes
				CanvasUpdate(gamepanelHandle, PANEL_4_CANVAS, VAL_ENTIRE_OBJECT);
				CanvasEndBatchDraw (gamepanelHandle, PANEL_4_CANVAS);
			}
		}
		CmtReleaseLock(lockhandle);
		Sleep(10);
	}
	return 0;
}

//this function is the thread that hendels the cursor locttion and sets it as the player location as well as handling palyer shooting
int CVICALLBACK drawplayer_treadfunc(void *functionData)
{
	int success;
	const int projectile_speed = 3;  // Speed at which the projectile moves
	int size = sizeof(playerProjectiles) / sizeof(playerProjectiles[0]);
	static time_t lastShotTimes[10] = {0};  // Track last shot time for each projectile
	while (player_thread->status)
	{
		CmtGetLockEx(lockhandle, 0, CMT_WAIT_FOREVER, &success);

		// Only proceed if the lock was acquired successfully
		if (success)
		{
			// Handle pause and game over states
			if (isKeyPressed(VK_ESCAPE))
			{
				pause = 1;
				DisplayPanel(pausepanelHandle);
			}
			while (pause)
			{
				if (success)
				{
					CmtReleaseLock(lockhandle);  // Release lock while paused
					success = 0;
				}
				ProcessSystemEvents();
				if (unpause)
					CmtGetLockEx(lockhandle, 0, CMT_WAIT_FOREVER, &success);  // Reacquire lock
			}

			//////////////////////////////////////////////////////////////////////////
			// in the code sagment below, the palyer's score continuesly updates.	//
			// later on there is a check to see if the player has lot all his lives //
			// if so that means the game is over, the user is transfered to the 	//
			// next window where he will see his score and retur to the main menu.  //
			//////////////////////////////////////////////////////////////////////////
			CanvasStartBatchDraw (gamepanelHandle, PANEL_4_CANVAS);
			// Clear and redraw background
			CanvasClear(gamepanelHandle, PANEL_4_CANVAS, VAL_ENTIRE_OBJECT);
			CanvasDrawBitmap(gamepanelHandle, PANEL_4_CANVAS, background_bitmapid, VAL_ENTIRE_OBJECT, VAL_ENTIRE_OBJECT);
			// Display player score and health
			snprintf(scoreText, sizeof(scoreText), "SCORE: %d", player.score);
			CanvasDrawRect(gamepanelHandle, PANEL_4_CANVAS, MakeRect(0, 0, 30, 100), VAL_DRAW_INTERIOR);
			SetCtrlAttribute(gamepanelHandle, PANEL_4_CANVAS, ATTR_PEN_FILL_COLOR, VAL_DK_BLUE);
			SetCtrlAttribute(gamepanelHandle, PANEL_4_CANVAS, ATTR_PEN_COLOR, VAL_WHITE);
			CanvasDrawText(gamepanelHandle, PANEL_4_CANVAS, scoreText, VAL_APP_META_FONT, MakeRect(0, 0, 30, 100), VAL_CENTER);
			// Check if player is dead
			if (player.health == 0)
			{
				pause = 1;
				DisplayPanel(GOpanelHandle);
				if (sound_flag)
				{
					BASS_ChannelPause(background_theme_music);
					BASS_ChannelSetAttribute(gameover_sound, BASS_ATTRIB_VOL, 0.1);
					BASS_ChannelPlay(gameover_sound, TRUE);
				}
			}
			// Update health display
			for (int i = 1; i <= player.health; i++)
				CanvasDrawBitmap(gamepanelHandle, PANEL_4_CANVAS, heart_bitmapid, VAL_ENTIRE_OBJECT, MakeRect(0, 1000 - 30 * i, 30, 30));

			///////////////////////////////////////////////////////////////////////////////////
			// Handle player movement and firing, depending on input method                  //
			// here according to the player's choise the x coordinates will be updated 		 //
			// via the mouse, each time the mouse or the button on the controller is pressed //
			// 'fire_butt' will triger the firing mechnism.									 //
			// and mouse movment will be updated to player x coordinates. 					 //
			///////////////////////////////////////////////////////////////////////////////////
			switch (cont)
			{
				case 0:  // Mouse control
					GetRelativeMouseState(gamepanelHandle, 0, &player.x, NULL, &fire_butt, NULL, NULL);
					break;

				case 1:  // Arduino controller
					GetRelativeMouseState(gamepanelHandle, 0, &player.x, NULL, NULL, NULL, NULL);
					char buffer[20];   // Make the buffer larger to hold multiple fragments
					static int bytesReadTotal = 0;  // Keep track of the total bytes read so far
					int bytesRead = ComRd(4, buffer + bytesReadTotal, sizeof(buffer) - bytesReadTotal - 1);  // Read into the buffer, starting where we left off
					buffer[bytesRead - 1] = '\0';  // Null-terminate the string

					// Process button press
					if (strstr(buffer, "y"))
						fire_butt = 1;
					else if (strstr(buffer, "n"))
						fire_butt = 0;
					break;
			}

			/////////////////////////////////////////////////////////////////
			// 						   draw player						   //
			// this sagment handels the player's loction				   //
			// based on the function above that updates the 			   //
			// x coordinates of the player, this sagment checks that there //
			// is no deviation from the canves bounderies, and draws the   //
			// player in the corect new position.						   //
			/////////////////////////////////////////////////////////////////
			if (player.x < 0)
				player.x = 0;
			if (player.x > 950)
				player.x = 950;
			// Draw player at new location
			Rect player_rect = MakeRect(600, player.x, 50, 50);
			CanvasDrawBitmap(gamepanelHandle, PANEL_4_CANVAS, player_bitmapid, VAL_ENTIRE_OBJECT, player_rect);

			////////////////////////////////////////////////////////////////////////////////
			//             					  Handle shooting							  //
			// the player is allowed to have only 7 active projectile active at once. 	  //
			// 							   make every shot count!						  //
			// in addition to that the user may fire a projectile once every 0.5 secondes //
			// the firing machanism when activated draws the projectile from the center   //
			// of the player's rectangle and for each iteration advances it 			  //
			// relativly to its velocity												  //
			////////////////////////////////////////////////////////////////////////////////
			time_t currentTime = time(NULL);
			if (fire_butt && difftime(currentTime, lastShotTimes[0]) > 0.5)  // Ensure delay between shots
			{
				for (int i = 0; i < size; i++)
					if (!playerProjectiles[i].active)
					{
						if(sound_flag)
						{
							BASS_ChannelSetAttribute(player_fire_sound, BASS_ATTRIB_VOL, 0.05);
							BASS_ChannelPlay(player_fire_sound, TRUE);
						}
						playerProjectiles[i].x = player.x + 17;
						playerProjectiles[i].y = 600;
						playerProjectiles[i].active = 1;
						lastShotTimes[0] = currentTime;  // Update shot time
						break;
					}
				fire_butt = 0;  // Reset fire button
			}
			for (int i = 0; i < size; i++)
				if (playerProjectiles[i].active)
				{
					playerProjectiles[i].y -= projectile_speed;  // Move projectile upwards
					// Deactivate projectile if it reaches the top
					if (playerProjectiles[i].y <= 0)
						playerProjectiles[i].active = 0;
					else
					{
						Rect projectile_rect = MakeRect(playerProjectiles[i].y, playerProjectiles[i].x, 20, 16);
						CanvasDrawBitmap(gamepanelHandle, PANEL_4_CANVAS, hero_laser, VAL_ENTIRE_OBJECT, projectile_rect);
					}
				}

			// Update the entire canvas to reflect changes
			CanvasUpdate(gamepanelHandle, PANEL_4_CANVAS, VAL_ENTIRE_OBJECT);
			CanvasEndBatchDraw (gamepanelHandle, PANEL_4_CANVAS);
		}
		CmtReleaseLock(lockhandle);
		Sleep(10);
	}
	return 0;
}

//displays chosen statistics
int CVICALLBACK displaybuttfunc (int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
	static double score_arr[100], diff[100];
	int count = 0;  // Count of how many rows read
	FILE *file;
	switch (event)
	{
		case EVENT_COMMIT:
			DisplayPanel(statspanelHandle);
			// Open the file and read data into arrays
			file = fopen("data.csv", "r");
			if (file == NULL)
			{
				printf("Error opening file.\n");
				return -1;
			}
			// Read data from the CSV file
			int score, difficulty;
			count = 0;
			while (fscanf(file, "%d,%d", &difficulty, &score) == 2 && count < 100)
			{
				diff[count] = difficulty;
				score_arr[count] = score;
				count++;
			}
			fclose(file);  // Close the file after reading
			// Sort the scores in descending order
			for (int i = 0; i < count - 1; i++)
				for (int j = i + 1; j < count; j++)
					if (score_arr[i] < score_arr[j])
					{
						// Swap scores
						int temp_score = score_arr[i];
						score_arr[i] = score_arr[j];
						score_arr[j] = temp_score;

						// Swap corresponding difficulties
						int temp_diff = diff[i];
						diff[i] = diff[j];
						diff[j] = temp_diff;
					}
			
			DeleteGraphPlot (statspanelHandle, PANEL_7_GRAPH, -1, VAL_IMMEDIATE_DRAW);
			double x_vals[10];  // Store X values (indices)
			double y_vals[10];  // Store Y values (scores)
			for (int i = 0; i < 10 && i < count; i++)
			{
				x_vals[i] = i + 1;  // X axis shows ranking (1 to 10)
				y_vals[i] = score_arr[i];  // Y axis shows the score
			}
			SetAxisScalingMode (statspanelHandle, PANEL_7_GRAPH, VAL_BOTTOM_XAXIS, VAL_MANUAL,0.0, 11.0);
			PlotXY (statspanelHandle, PANEL_7_GRAPH, x_vals, y_vals, 10, VAL_DOUBLE, VAL_DOUBLE, VAL_VERTICAL_BAR, VAL_NO_POINT, VAL_SOLID, 1, VAL_GREEN);
	}

	return 0;
}

//Menu quit shortcut
void CVICALLBACK QuitMenufunc (int menuBar, int menuItem, void *callbackData, int panel)
{
	exitbutfunc(0,0,EVENT_COMMIT,NULL,0,0);
}

//Unpause the game
int CVICALLBACK resumebuttfunc (int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			pause = 0;
			unpause = 1;
			HidePanel (pausepanelHandle);
			DisplayPanel (gamepanelHandle);
			break;
	}
	return 0;
}

//GameOver mechanism
int CVICALLBACK funkk (int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_GOT_FOCUS:
			// Display the player's score on the game over screen
			snprintf(scoreText, sizeof(scoreText), "SCORE: %d", player.score);
			CanvasDrawRect(GOpanelHandle, PANEL_6_CANVAS, MakeRect(100, 100, 30, 100), VAL_DRAW_INTERIOR);
			SetCtrlAttribute(GOpanelHandle, PANEL_6_CANVAS, ATTR_PEN_FILL_COLOR, VAL_DK_BLUE);
			SetCtrlAttribute(GOpanelHandle, PANEL_6_CANVAS, ATTR_PEN_COLOR, VAL_WHITE);
			CanvasDrawText(GOpanelHandle, PANEL_6_CANVAS, scoreText, VAL_APP_META_FONT, MakeRect(100, 100, 30, 100), VAL_CENTER);
			SavePlayerStatsToCSV(player.score, difficulty_factor);
			pause = 0;
			player.score = 0;
			player.health = -1;
			player_thread->status = 0;
			enemy_thread->status = 0;
			damage_thread->status = 0;
			break;
		case EVENT_COMMIT:
			if (sound_flag)
				BASS_ChannelPause(gameover_sound);
			pause = 0;
			HidePanel (gamepanelHandle);
			HidePanel(GOpanelHandle);
			DisplayPanel(mainpanelHandle);
			break;
	}
	return 0;
}


int CVICALLBACK savebuttfunc (int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			SavePlayerStatsToCSV(player.score, difficulty_factor);
			break;
	}
	return 0;
}

//return from pause window to main window
int CVICALLBACK rtnpausemainfunc (int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			HidePanel (pausepanelHandle);
			HidePanel (gamepanelHandle);
			DisplayPanel (mainpanelHandle);
			break;
	}
	return 0;
}

void SavePlayerStatsToCSV(int score, int difficulty)
{
	FILE *file;
	file = fopen("data.csv", "a");
	// Write data to CSV file (e.g., Date, Time, Difficulty, Score)
	fprintf(file, "%d, %d\n", difficulty, player.score);
	// Close the file
	fclose(file);
}

void CVICALLBACK sound_enabler (int menuBar, int menuItem, void *callbackData, int panel)
{
	sound_flag = 0;
	BASS_ChannelPause(background_theme_music);
}

void CVICALLBACK highscorefunc (int menuBar, int menuItem, void *callbackData, int panel)
{
	DisplayPanel (statisticspanelHandle);
}

int CVICALLBACK rtrnstat2 (int panel, int control, int event,
						   void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			HidePanel(statspanelHandle);
			break;
	}
	return 0;
}

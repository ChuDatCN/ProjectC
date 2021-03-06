#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <wchar.h>
#include "inc/func/soundex.h"
#include "inc/func/tab.h"
#include "inc/func/wordwrap.h"
#include "inc/func/convert.h"
#include "inc/CSFML/include/SFML/Graphics.h"
#include "inc/btree/inc/btree.h"

#define BH 40
#define BW 100
#define LBH 40
#define LBW 400
#define WW 800
#define WH 600
#define FONT "font/arial.ttf"
#define BACKGROUND "img/background.png"
#define TITLE "img/title.png"
#define BUTTON "img/button.png"
#define LONGBUTTON "img/longbutton.png"
#define ICON "img/icon.png"
#define DATA "data/data.btree"
#define SOUNDEX "data/soundex.btree"

#define ifpos(pos, bw, bh) \
  if (sfMouse_getPosition((sfWindow *)window).x >= pos.x &&      \  
      sfMouse_getPosition((sfWindow *)window).y >= pos.y &&      \
      sfMouse_getPosition((sfWindow *)window).x <= pos.x + bw && \
      sfMouse_getPosition((sfWindow *)window).y <= pos.y + bh)

#define iftextinput(word, lengh) if (event2.text.unicode < 127 && event2.text.unicode > 32){ \
          if (strlen(word) < lengh){ \
            word[strlen(word) + 1] = 0; \
            if (event2.text.unicode >= 'A' && event2.text.unicode <= 'Z') \
              word[strlen(word)] = (char)(event2.text.unicode + 32); \
            else word[strlen(word)] = (char)event2.text.unicode;}} \
        else if (event2.text.unicode == ' ') { \
          if (strlen(word) < lengh){ \
            word[strlen(word) + 1] = 0; \
            word[strlen(word)] = (char)event2.text.unicode;}} \
        else if (event2.text.unicode == '\b' || event2.text.unicode == 127){ \
          if (strlen(word) > 0) \
            word[strlen(word) - 1] = 0;}

/* Global resources */
sfVector2f back_pos = {30, 30};
sfVector2f home_pos = {670, 30};
sfVector2f next_pos = {670, 530};
sfVector2f prev_pos = {30, 530};
sfVector2f center_bt_pos = {350, 530};
sfTexture *button;
sfTexture *long_button;
sfTexture *bg;
sfFont *font;
sfSprite *button_home;
sfSprite *button_back;
sfSprite *button_next;
sfSprite *button_prev;
sfSprite *button_center;
sfSprite *background;
sfVector2f text_pos = {100, 75};
sfText *text_word;
sfText *back_bt;
sfText *home_bt;
sfText *next_bt;
sfText *prev_bt;
sfText *center_bt;
sfText *title_word;
/* Btree */
BTA *dt, *sd;

int textureInit();
sfVector2f button_create(sfSprite *bt, float x, float y, sfTexture *texture);
void button_text(sfText *text, sfVector2f bt_pos, float bt_width, float bt_height, char *string);
void title_text(sfText *text, char *string);
void text_attribute(sfText *text, sfFont *font, sfColor color, int size, sfVector2f pos);
void destroy_globalTexture();
int searchScreen(sfRenderWindow *window);
int translateScreen(sfRenderWindow *window, unsigned char *word, char *word_en);
int translateError(sfRenderWindow *window, char *word, char suggest[5][100], int suggest_count);
void translateError_clean(sfText *text1, sfText *text2, sfText *array_text[5], sfSprite *array_sprite[5]);
int addWordScreen(sfRenderWindow *window);
int addScreen(sfRenderWindow *window, char *word_en);
int addError(sfRenderWindow *window, char *word);
void addSuccess(sfRenderWindow *window, char *word);
int delScreen(sfRenderWindow *window);
int delError(sfRenderWindow *window, char *word);
void delSuccess(sfRenderWindow *window, char *word);
void aboutScreen(sfRenderWindow *window);

int main()
{
  btinit();
  init();
  dt = btopn(DATA, FALSE, FALSE); /* Open data Btree */
  sd = btopn(SOUNDEX, FALSE, FALSE);/* Open soundex Btree */
  if (!dt || !sd)
  {
    printf("Missing data.\n");
    return 1;
  }
  /* Icon */
  sfImage *icon = sfImage_createFromFile(ICON);
  /* Title */
  sfTexture *title = sfTexture_createFromFile(TITLE, NULL);
  if(!title || !icon){
    printf("Missing resources.\n");
    return 1;
  }
  sfSprite *title_s = sfSprite_create();
  sfSprite_setTexture(title_s, title, sfTrue);
  /* Text and Texture */
  if(!textureInit()){
    printf("Missing resources.\n");
    return 1;
  }
  /* Make window */
  sfVideoMode video = {WW, WH, 32};
  sfRenderWindow *window;
  sfEvent event;
  window = sfRenderWindow_create(video, "E-V Dictionary", sfClose, NULL);/* window name */
  sfRenderWindow_setFramerateLimit(window, 60);                         /* fps limit */
  sfRenderWindow_setIcon(window, 200, 200, sfImage_getPixelsPtr(icon)); /* app icon */
  if (!window)
    return 1;
  /* Button */
  /* Search */
  sfSprite *button_tl = sfSprite_create();
  sfVector2f tl_pos = button_create(button_tl, 600, 100, button);
  /* Search text */
  sfText *search_bt = sfText_create();
  button_text(search_bt, tl_pos, BW, BH, "Search");
  /* Add */
  sfSprite *button_add = sfSprite_create();
  sfVector2f add_pos = button_create(button_add, 600, 190, button);
  sfText *add_bt = sfText_create();
  button_text(add_bt, add_pos, BW, BH, "Add");
  /* Delete */
  sfSprite *button_del = sfSprite_create();
  sfVector2f del_pos = button_create(button_del, 600, 280, button);
  sfText *del_bt = sfText_create();
  button_text(del_bt, del_pos, BW, BH, "Delete");
  /* Credit */
  sfSprite *button_about = sfSprite_create();
  sfVector2f about_pos = button_create(button_about, 600, 370, button);
  sfText *about_bt = sfText_create();
  button_text(about_bt, about_pos, BW, BH, "About");
  /* Close */
  sfSprite *button_exit = sfSprite_create();
  sfVector2f exit_pos = button_create(button_exit, 600, 460, button);
  sfText *exit_bt = sfText_create();
  button_text(exit_bt, exit_pos, BW, BH, "Exit");
  /* Event */
  while (sfRenderWindow_isOpen(window))
  {
    /* Display */
    sfRenderWindow_clear(window, sfWhite);
    sfRenderWindow_drawSprite(window, background, NULL);
    sfRenderWindow_drawSprite(window, title_s, NULL);
    sfRenderWindow_drawSprite(window, button_tl, NULL);
    sfRenderWindow_drawText(window, search_bt, NULL);
    sfRenderWindow_drawSprite(window, button_add, NULL);
    sfRenderWindow_drawText(window, add_bt, NULL);
    sfRenderWindow_drawSprite(window, button_del, NULL);
    sfRenderWindow_drawText(window, del_bt, NULL);
    sfRenderWindow_drawSprite(window, button_about, NULL);
    sfRenderWindow_drawText(window, about_bt, NULL);
    sfRenderWindow_drawSprite(window, button_exit, NULL);
    sfRenderWindow_drawText(window, exit_bt, NULL);
    sfRenderWindow_display(window);
    /* Process events */
    sfEvent event2;
    while (sfRenderWindow_pollEvent(window, &event2))
    {
      if (event2.type == sfEvtClosed)
        sfRenderWindow_close(window);
      if (event2.type == sfEvtMouseButtonPressed){
        ifpos(tl_pos, BW, BH)
          searchScreen(window);
        ifpos(add_pos, BW, BH)
          addWordScreen(window);
        ifpos(del_pos, BW, BH)
          delScreen(window);
        ifpos(about_pos, BW, BH)
          aboutScreen(window);
        ifpos(exit_pos, BW, BH)
          sfRenderWindow_close(window);
      }
    }
  }
  /* Clean-up */
  destroy_globalTexture();
  sfImage_destroy(icon);
  sfTexture_destroy(title);
  sfSprite_destroy(title_s);
  sfSprite_destroy(button_about);
  sfSprite_destroy(button_tl);
  sfSprite_destroy(button_add);
  sfSprite_destroy(button_del);
  sfSprite_destroy(button_exit);
  sfText_destroy(search_bt);
  sfText_destroy(add_bt);
  sfText_destroy(about_bt);
  sfText_destroy(del_bt);
  sfText_destroy(exit_bt);
  sfRenderWindow_destroy(window);
  return 0;
};

int searchScreen(sfRenderWindow *window)
{
  char *word = (char *)calloc(100, sizeof(char));
  char *search = "Search:";
  unsigned char val[10000];
  /* Text */
  sfVector2f word_pos = {100, 100};
  sfText *tl_word = sfText_create();
  char temp[100];
  char suggest[500];
  char suggest_text[5][100];
  int suggest_count;
  char word_sound[100];
  char temp_find[100];
  char suggest_print[10];
  int check_dup = 0;
  int t;
  char temp_sound[100];
  strcpy(temp, word);
  strcat(temp, "_");
  text_attribute(tl_word, font, sfBlack, 20, word_pos);
  /* Suggest */
  sfVector2f suggest_pos = {100, 250};
  sfText *suggest_word = sfText_create();
  text_attribute(suggest_word, font, sfBlack, 20, suggest_pos);
  /* Text */
  sfText_setString(text_word, search);
  /* Title */
  title_text(title_word, "Translate");
  /* Button */
  /* Translate */
  button_text(center_bt, center_bt_pos, BW, BH, "Translate");
  /* Display */
  while (sfRenderWindow_isOpen(window))
  {
    sfText_setString(tl_word, temp);
    /* Suggest text */
    if (strlen(word))
    {
      suggest_count = 0;
      custom_bfndky(dt, word, suggest_text[suggest_count]);/* find prefix word */
      sprintf(suggest, "Suggest:\n1. %s", suggest_text[suggest_count]);/* suggest text */
      suggest_count++;
      strcpy(word_sound, soundex(word));
      if (!btsel(sd, word_sound, suggest_text[suggest_count], 100, &t))/* find 1 soundex word */
      {
        check_dup = 0;
        for (int i = 0; i < suggest_count; i++)
        {
          if (!strcmp(suggest_text[suggest_count], suggest_text[i]))
          {
            check_dup++;
            break;
          }
        }
        if (!check_dup)
        {
          strcat(suggest, "\n2. ");
          strcat(suggest, suggest_text[suggest_count]);
          suggest_count++;
        }
        while (suggest_count < 5)/* find 4 more */
        {
          btseln(sd, temp_sound, suggest_text[suggest_count], 100, &t);
          if (!strcmp(word_sound, temp_sound))
          {
            check_dup = 0;
            for (int i = 0; i < suggest_count; i++)
            {
              if (!strcmp(suggest_text[suggest_count], suggest_text[i]))
              {
                check_dup++;
                break;
              }
            }
            if (!check_dup)
            {
              sprintf(suggest_print, "\n%d. ", suggest_count + 1);
              strcat(suggest, suggest_print);
              strcat(suggest, suggest_text[suggest_count]);
              suggest_count++;
            }
          }
          else
            break;
        }
      }
    }
    else
      suggest[0] = '\0';
    sfText_setString(suggest_word, suggest);
    /* Process events */
    sfRenderWindow_clear(window, sfWhite);
    sfRenderWindow_drawSprite(window, background, NULL);
    sfRenderWindow_drawSprite(window, button_center, NULL);
    sfRenderWindow_drawText(window, center_bt, NULL);
    sfRenderWindow_drawSprite(window, button_back, NULL);
    sfRenderWindow_drawText(window, back_bt, NULL);
    sfRenderWindow_drawSprite(window, button_home, NULL);
    sfRenderWindow_drawText(window, home_bt, NULL);
    sfRenderWindow_drawText(window, tl_word, NULL);
    sfRenderWindow_drawText(window, suggest_word, NULL);
    sfRenderWindow_drawText(window, title_word, NULL);
    sfRenderWindow_drawText(window, text_word, NULL);
    sfRenderWindow_display(window);
    sfEvent event2;
    while (sfRenderWindow_pollEvent(window, &event2))
    {
      if (event2.type == sfEvtClosed)
        sfRenderWindow_close(window);
      if (sfKeyboard_isKeyPressed(sfKeyLControl) || sfKeyboard_isKeyPressed(sfKeyRControl))/* Ctrl + num event */
      {
        if (event2.type == sfEvtKeyPressed)
        {
          if (event2.key.code >= 1 + sfKeyNum0 && event2.key.code <= suggest_count + sfKeyNum0)
            strcpy(word, suggest_text[event2.key.code - sfKeyNum1]);
        }
      }
      else
      {
        if (event2.type == sfEvtTextEntered)
        {
          iftextinput(word, 99)
          else if (event2.text.unicode == '\t')
            tabcomplete(dt, word);
          if (event2.text.unicode == '\r')/* translate when press Enter */
          {
            if (!btsel(dt, word, (char *)val, sizeof(val), &t))
            {
              if (!translateScreen(window, val, word))
              {
                free(word);
                sfText_destroy(tl_word);
                sfText_destroy(suggest_word);
                return 0;
              }
            }
            else
            {
              if (!translateError(window, word, suggest_text, suggest_count))
              {
                sfText_destroy(tl_word);
                sfText_destroy(suggest_word);
                free(word);
                return 0;
              }
            }
          }
        }
      }
      strcpy(temp, word);
      strcat(temp, "_");
      if (event2.type == sfEvtMouseButtonPressed)
      {
        ifpos(back_pos, BW, BH)
        {
          sfText_destroy(tl_word);
          sfText_destroy(suggest_word);
          return 1;
        }
        ifpos(home_pos, BW, BH)
        {
          sfText_destroy(tl_word);
          sfText_destroy(suggest_word);
          return 0;
        }
        ifpos(center_bt_pos, BW, BH)
        {
          int ch;
          if (!btsel(dt, word, (char *)val, sizeof(val), &t))
          {
            ch = translateScreen(window, val, word);
            if (!ch)
            {
              sfText_destroy(tl_word);
              sfText_destroy(suggest_word);
              return 0;
            }
            else
              continue;
          }
          else
          {
            ch = translateError(window, word, suggest_text, suggest_count);
            if (!ch)
            {
              sfText_destroy(tl_word);
              sfText_destroy(suggest_word);
              return 0;
            }
            else
              continue;
          }
        }
      }
    }
  }
};

int translateScreen(sfRenderWindow *window, unsigned char *word, char *word_en)
{
  wchar_t tempw[10000] = L"";
  UTF8_to_UTF32(word, tempw); /* convert utf8 to utf32 */
  wchar_t word_page[MAX_PAGE][2000];
  int page;
  page = wordwrap(tempw, word_page); /*  */
  int current_page = 0;
  /* Text */
  sfVector2f tl_pos = {100, 100};
  sfText *tl_word = sfText_create();
  text_attribute(tl_word, font, sfBlack, 20, tl_pos);
  /* Text */
  char translate[100];
  sprintf(translate, "%s:", word_en);
  sfText_setString(text_word, translate);
  /* Title */
  title_text(title_word, "Translate");
  /* Display */
  while (sfRenderWindow_isOpen(window))
  {
    sfRenderWindow_clear(window, sfWhite);
    sfRenderWindow_drawSprite(window, background, NULL);
    if (page > 0 && current_page < page)
    {
      sfRenderWindow_drawSprite(window, button_next, NULL);
      sfRenderWindow_drawText(window, next_bt, NULL);
    }
    if (page > 0 && current_page > 0)
    {
      sfRenderWindow_drawSprite(window, button_prev, NULL);
      sfRenderWindow_drawText(window, prev_bt, NULL);
    }
    sfRenderWindow_drawSprite(window, button_back, NULL);
    sfRenderWindow_drawText(window, back_bt, NULL);
    sfRenderWindow_drawSprite(window, button_home, NULL);
    sfRenderWindow_drawText(window, home_bt, NULL);
    sfText_setUnicodeString(tl_word, word_page[current_page]);
    sfRenderWindow_drawText(window, tl_word, NULL);
    sfRenderWindow_drawText(window, title_word, NULL);
    sfRenderWindow_drawText(window, text_word, NULL);
    sfRenderWindow_display(window);
    /* Process events */
    sfEvent event2;
    while (sfRenderWindow_pollEvent(window, &event2))
    {
      if (event2.type == sfEvtClosed)
        sfRenderWindow_close(window);
      if (event2.type == sfEvtMouseButtonPressed)
      {
        ifpos(back_pos, BW, BH)
        {
          sfText_destroy(tl_word);
          return 1;
        }
        ifpos(home_pos, BW, BH)
        {
          sfText_destroy(tl_word);
          return 0;
        }
        ifpos(next_pos, BW, BH) if (current_page < page)
            current_page++;
        ifpos(prev_pos, BW, BH) if (current_page > 0)
            current_page--;
      }
    }
  }
};

int translateError(sfRenderWindow *window, char *word, char suggest[5][100], int suggest_count)
{
  char error_word[100];
  sprintf(error_word, "Error:\n\t\"%s\" doesn't exist!", word);
  /* Text */
  sfVector2f error_pos = {100, 100};
  sfText *error = sfText_create();
  sfText_setString(error, error_word);
  text_attribute(error, font, sfBlack, 20, error_pos);
  /* Title */
  title_text(title_word, "Translate");
  /* Button */
  /* Suggest */
  sfSprite *button_suggest[5];
  sfText *suggest_bt[5];
  sfVector2f suggest_pos[5];
  sfText *suggest_text = sfText_create();
  sfVector2f suggest_text_pos = {100, 170};
  char *suggest_word = "Do you mean:";
  sfText_setString(suggest_text, suggest_word);
  text_attribute(suggest_text, font, sfBlack, 20, suggest_text_pos);
  for (int i = 0; i < suggest_count; i++)
  {
    suggest_pos[i].x = 200;
    suggest_pos[i].y = 200 + 50 * i;
    button_suggest[i] = sfSprite_create();
    suggest_bt[i] = sfText_create();
    button_create(button_suggest[i], suggest_pos[i].x, suggest_pos[i].y, long_button);
    button_text(suggest_bt[i], suggest_pos[i], LBW, LBH, suggest[i]);
  }
  /* Add */
  button_text(center_bt, center_bt_pos, BW, BH, "Add");
  sfText *add_text = sfText_create();
  sfVector2f add_text_pos = {100, 500};
  char add_word[200];
  sprintf(add_word, "Or add \"%s\" to database:", word);
  sfText_setString(add_text, add_word);
  text_attribute(add_text, font, sfBlack, 20, add_text_pos);
  /* Display */
  while (sfRenderWindow_isOpen(window))
  {
    /* Process events */
    sfRenderWindow_clear(window, sfWhite);
    sfRenderWindow_drawSprite(window, background, NULL);
    sfRenderWindow_drawSprite(window, button_back, NULL);
    sfRenderWindow_drawText(window, back_bt, NULL);
    sfRenderWindow_drawSprite(window, button_home, NULL);
    sfRenderWindow_drawText(window, home_bt, NULL);
    sfRenderWindow_drawText(window, error, NULL);
    sfRenderWindow_drawText(window, title_word, NULL);
    sfRenderWindow_drawSprite(window, button_center, NULL);
    sfRenderWindow_drawText(window, center_bt, NULL);
    for (int i = 0; i < suggest_count; i++)
    {
      sfRenderWindow_drawSprite(window, button_suggest[i], NULL);
      sfRenderWindow_drawText(window, suggest_bt[i], NULL);
    }
    if (suggest_count)
    {
      sfRenderWindow_drawText(window, suggest_text, NULL);
    }
    sfRenderWindow_drawText(window, add_text, NULL);
    sfRenderWindow_display(window);
    sfEvent event2;
    while (sfRenderWindow_pollEvent(window, &event2))
    {
      if (event2.type == sfEvtClosed)
        sfRenderWindow_close(window);
      if (event2.type == sfEvtMouseButtonPressed)
      {
        ifpos(home_pos, BW, BH)
        {
          translateError_clean(error, suggest_text, suggest_bt, button_suggest);
          return 0;
        }
        ifpos(back_pos, BW, BH)
        {
          translateError_clean(error, suggest_text, suggest_bt, button_suggest);
          return 1;
        }
        ifpos(center_bt_pos, BW, BH) if (!addScreen(window, word)) return 0;
        for (int i = 0; i < suggest_count; i++)
        {
          ifpos(suggest_pos[i], LBW, LBH)
          {
            unsigned char word_mean[10000];
            int t;
            btsel(dt, suggest[i], word_mean, 10000, &t);
            if (!translateScreen(window, word_mean, suggest[i]))
            {
              translateError_clean(error, suggest_text, suggest_bt, button_suggest);
              return 0;
            }
          }
        }
      }
    }
  }
}

void translateError_clean(sfText *text1, sfText *text2, sfText *array_text[5], sfSprite *array_sprite[5])
{
  sfText_destroy(text1);
  sfText_destroy(text2);
  for (int i = 0; i < 5; i++)
  {
    sfSprite_destroy(array_sprite[i]);
    sfText_destroy(array_text[i]);
  }
}

int addWordScreen(sfRenderWindow *window)
{
  char *word = (char *)calloc(100, sizeof(char));
  /* Text */
  sfVector2f word_pos = {100, 100};
  sfText *add_word = sfText_create();
  char temp[100];
  int t;
  strcpy(temp, word);
  strcat(temp, "_");
  text_attribute(add_word, font, sfBlack, 20, word_pos);
  /* Text */
  char *add = "Add:";
  sfText_setString(text_word, add);
  /* Title */
  title_text(title_word, "Add");
  /* Button */
  /* Add */
  button_text(center_bt, center_bt_pos, BW, BH, "Add");
  /* Display */
  while (sfRenderWindow_isOpen(window))
  {
    sfText_setString(add_word, temp);
    /* Process events */
    sfRenderWindow_clear(window, sfWhite);
    sfRenderWindow_drawSprite(window, background, NULL);
    sfRenderWindow_drawSprite(window, button_center, NULL);
    sfRenderWindow_drawText(window, center_bt, NULL);
    sfRenderWindow_drawSprite(window, button_back, NULL);
    sfRenderWindow_drawText(window, back_bt, NULL);
    sfRenderWindow_drawSprite(window, button_home, NULL);
    sfRenderWindow_drawText(window, home_bt, NULL);
    sfRenderWindow_drawText(window, add_word, NULL);
    sfRenderWindow_drawText(window, title_word, NULL);
    sfRenderWindow_drawText(window, text_word, NULL);
    sfRenderWindow_display(window);
    sfEvent event2;
    while (sfRenderWindow_pollEvent(window, &event2))
    {
      if (event2.type == sfEvtClosed)
        sfRenderWindow_close(window);
      if (event2.type == sfEvtTextEntered)
      {
        iftextinput(word, 99);
        if (event2.text.unicode == '\r')
        {
          if (bfndky(dt, word, &t))
          {
            if (!addScreen(window, word))
            {
              sfText_destroy(add_word);
              free(word);
              return 0;
            }
          }
          else
          {
            if (!addError(window, word))
            {
              sfText_destroy(add_word);
              free(word);
              return 0;
            }
          }
        }
        strcpy(temp, word);
        strcat(temp, "_");
      }
      if (event2.type == sfEvtMouseButtonPressed)
      {
        ifpos(back_pos, BW, BH)
        {
          sfText_destroy(add_word);
          free(word);
          return 1;
        }
        ifpos(home_pos, BW, BH)
        {
          sfText_destroy(add_word);
          free(word);
          return 0;
        }
        ifpos(center_bt_pos, BW, BH)
        {
          int ch;
          if (bfndky(dt, word, &t))
          {
            ch = addScreen(window, word);
            if (!ch)
            {
              free(word);
              sfText_destroy(add_word);
              return 0;
            }
            else
              continue;
          }
          else
          {
            ch = addError(window, word);
            if (!ch)
            {
              free(word);
              sfText_destroy(add_word);
              return 0;
            }
            else
              continue;
          }
        }
      }
    }
  }
};

int addScreen(sfRenderWindow *window, char *word_en)
{
  int line_count = 0;
  char *word = (char *)calloc(10000, sizeof(char));
  /* Text */
  sfVector2f word_pos = {100, 100};
  sfText *add_word = sfText_create();
  char temp[10000];
  int t;
  strcpy(temp, word);
  strcat(temp, "_");
  text_attribute(add_word, font, sfBlack, 20, word_pos);
  /* Text */
  char add[100];
  sprintf(add, "%s:", word_en);
  sfText_setString(text_word, add);
  /* Title */
  title_text(title_word, "Add");
  /* Button */
  /* Add */
  button_text(center_bt, center_bt_pos, BW, BH, "Add");
  /* Display */
  while (sfRenderWindow_isOpen(window))
  {
    sfText_setString(add_word, temp);
    /* Process events */
    sfRenderWindow_clear(window, sfWhite);
    sfRenderWindow_drawSprite(window, background, NULL);
    sfRenderWindow_drawSprite(window, button_center, NULL);
    sfRenderWindow_drawText(window, center_bt, NULL);
    sfRenderWindow_drawSprite(window, button_back, NULL);
    sfRenderWindow_drawText(window, back_bt, NULL);
    sfRenderWindow_drawSprite(window, button_home, NULL);
    sfRenderWindow_drawText(window, home_bt, NULL);
    sfRenderWindow_drawText(window, add_word, NULL);
    sfRenderWindow_drawText(window, title_word, NULL);
    sfRenderWindow_drawText(window, text_word, NULL);
    sfRenderWindow_display(window);
    sfEvent event2;
    while (sfRenderWindow_pollEvent(window, &event2))
    {
      if (event2.type == sfEvtClosed)
        sfRenderWindow_close(window);
      if (event2.type == sfEvtTextEntered)
      {
        if (event2.text.unicode < 127 && event2.text.unicode > 32)
        {
          if (strlen(word) < 10000 && line_count < LINE)
          {
            word[strlen(word) + 1] = 0;
            if (event2.text.unicode >= 'A' && event2.text.unicode <= 'Z')
              word[strlen(word)] = (char)(event2.text.unicode + 32);
            else
              word[strlen(word)] = (char)event2.text.unicode;
          }
        }
        else if (event2.text.unicode == ' ')
        {
          if (strlen(word) < 10000 && line_count < LINE)
          {
            word[strlen(word) + 1] = 0;
            word[strlen(word)] = (char)event2.text.unicode;
          }
        }
        else if (event2.text.unicode == '\b' || event2.text.unicode == 127)
        {
          if (strlen(word) > 0)
          {
            if (word[strlen(word) - 1] == '\n')
              line_count--;
            word[strlen(word) - 1] = 0;
          }
        }
        if (event2.text.unicode == '\r')
        {
          if (strlen(word) < 10000 && line_count < LINE)
          {
            word[strlen(word) + 1] = 0;
            word[strlen(word)] = '\n';
            line_count++;
          }
        }
        line_count += add_wordwrap(word);
        strcpy(temp, word);
        strcat(temp, "_");
      }
      if (event2.type == sfEvtMouseButtonPressed)
      {
        ifpos(back_pos, BW, BH)
        {
          free(word);
          sfText_destroy(add_word);
          return 1;
        }
        ifpos(home_pos, BW, BH)
        {
          free(word);
          sfText_destroy(add_word);
          return 0;
        }
        ifpos(center_bt_pos, BW, BH)
        {
          btins(dt, word_en, word, strlen(word) + 1);
          char *sound_temp = (char *)malloc(100);
          strcpy(sound_temp, soundex(word_en));
          btins(sd, sound_temp, word_en, strlen(word_en) + 1);
          free(sound_temp);
          addSuccess(window, word_en);
          free(word);
          sfText_destroy(add_word);
          return 0;
        }
      }
    }
  }
};

int addError(sfRenderWindow *window, char *word)
{
  char error_word[100];
  sprintf(error_word, "Error:\n\t\"%s\" exist!", word);
  /* Text */
  sfVector2f error_pos = {100, 100};
  sfText *error = sfText_create();
  sfText_setString(error, error_word);
  text_attribute(error, font, sfBlack, 20, error_pos);
  /* Title */
  title_text(title_word, "Add");
  /* Display */
  while (sfRenderWindow_isOpen(window))
  {
    /* Process events */
    sfRenderWindow_clear(window, sfWhite);
    sfRenderWindow_drawSprite(window, background, NULL);
    sfRenderWindow_drawSprite(window, button_back, NULL);
    sfRenderWindow_drawText(window, back_bt, NULL);
    sfRenderWindow_drawSprite(window, button_home, NULL);
    sfRenderWindow_drawText(window, home_bt, NULL);
    sfRenderWindow_drawText(window, error, NULL);
    sfRenderWindow_drawText(window, title_word, NULL);
    sfRenderWindow_display(window);
    sfEvent event2;
    while (sfRenderWindow_pollEvent(window, &event2))
    {
      if (event2.type == sfEvtClosed)
        sfRenderWindow_close(window);
      if (event2.type == sfEvtMouseButtonPressed)
      {
        ifpos(home_pos, BW, BH)
        {
          sfText_destroy(error);
          return 0;
        }
        ifpos(back_pos, BW, BH)
        {
          sfText_destroy(error);
          return 1;
        }
      }
    }
  }
};

void addSuccess(sfRenderWindow *window, char *word)
{
  char add_word[100];
  sprintf(add_word, "Added \"%s\" to database!", word);
  /* Text */
  sfVector2f add_pos = {100, 100};
  sfText *add = sfText_create();
  sfText_setString(add, add_word);
  text_attribute(add, font, sfBlack, 20, add_pos);
  /* Title */
  title_text(title_word, "Add");
  /* Button */
  /* Home */
  button_text(center_bt, center_bt_pos, BW, BH, "Home");
  /* Display */
  while (sfRenderWindow_isOpen(window))
  {
    /* Process events */
    sfRenderWindow_clear(window, sfWhite);
    sfRenderWindow_drawSprite(window, background, NULL);
    sfRenderWindow_drawSprite(window, button_center, NULL);
    sfRenderWindow_drawText(window, center_bt, NULL);
    sfRenderWindow_drawText(window, add, NULL);
    sfRenderWindow_drawText(window, title_word, NULL);
    sfRenderWindow_display(window);
    sfEvent event2;
    while (sfRenderWindow_pollEvent(window, &event2))
    {
      if (event2.type == sfEvtClosed)
        sfRenderWindow_close(window);
      if (event2.type == sfEvtMouseButtonPressed)
      {
        ifpos(center_bt_pos, BW, BH)
        {
          sfText_destroy(add);
          return;
        }
      }
    }
  }
};

int delScreen(sfRenderWindow *window)
{
  char *word = (char *)calloc(100, sizeof(char));
  /* Text */
  sfVector2f word_pos = {100, 100};
  sfText *del_word = sfText_create();
  char temp[100];
  int t;
  strcpy(temp, word);
  strcat(temp, "_");
  text_attribute(del_word, font, sfBlack, 20, word_pos);
  /* Text */
  char *del = "Delete:";
  sfText_setString(text_word, del);
  /* Title */
  title_text(title_word, "Delete");
  /* Button */
  /* Delete */
  button_text(center_bt, center_bt_pos, BW, BH, "Delete");
  /* Display */
  while (sfRenderWindow_isOpen(window))
  {
    sfText_setString(del_word, temp);
    /* Process events */
    sfRenderWindow_clear(window, sfWhite);
    sfRenderWindow_drawSprite(window, background, NULL);
    sfRenderWindow_drawSprite(window, button_center, NULL);
    sfRenderWindow_drawText(window, center_bt, NULL);
    sfRenderWindow_drawSprite(window, button_back, NULL);
    sfRenderWindow_drawText(window, back_bt, NULL);
    sfRenderWindow_drawSprite(window, button_home, NULL);
    sfRenderWindow_drawText(window, home_bt, NULL);
    sfRenderWindow_drawText(window, del_word, NULL);
    sfRenderWindow_drawText(window, title_word, NULL);
    sfRenderWindow_drawText(window, text_word, NULL);
    sfRenderWindow_display(window);
    sfEvent event2;
    while (sfRenderWindow_pollEvent(window, &event2))
    {
      if (event2.type == sfEvtClosed)
        sfRenderWindow_close(window);
      if (event2.type == sfEvtTextEntered)
      {
        iftextinput(word, 99)
        else if (event2.text.unicode == '\t')
          tabcomplete(dt, word);
        if (event2.text.unicode == '\r')
        {
          if (bfndky(dt, word, &t))
          {
            if (!delError(window, word))
            {
              sfText_destroy(del_word);
              free(word);
              return 0;
            }
          }
          else
          {
            btdel(dt, word);
            char temp[100];
            strcpy(temp, soundex(word));
            char find_word[100];
            int t;
            btsel(sd, temp, find_word, 100, &t);
            while (strcmp(word, find_word))
              btseln(sd, temp, find_word, 100, &t);
            btdel(sd, NULL);
            delSuccess(window, word);
            free(word);
            sfText_destroy(del_word);
            return 0;
          }
        }
        strcpy(temp, word);
        strcat(temp, "_");
      }
      if (event2.type == sfEvtMouseButtonPressed)
      {
        ifpos(back_pos, BW, BH)
        {
          sfText_destroy(del_word);
          return 1;
        }
        ifpos(home_pos, BW, BH)
        {
          sfText_destroy(del_word);
          return 0;
        }
        ifpos(center_bt_pos, BW, BH)
        {
          int ch;
          if (bfndky(dt, word, &t))
          {
            ch = delError(window, word);
            if (!ch)
            {
              free(word);
              sfText_destroy(del_word);
              return 0;
            }
            else
              continue;
          }
          else
          {
            btdel(dt, word);
            char temp[100];
            strcpy(temp, soundex(word));
            char find_word[100];
            int t;
            btsel(sd, temp, find_word, 100, &t);
            while (strcmp(word, find_word))
              btseln(sd, temp, find_word, 100, &t);
            btdel(sd, NULL);
            delSuccess(window, word);
            free(word);
            sfText_destroy(del_word);
            return 0;
          }
        }
      }
    }
  }
};

int delError(sfRenderWindow *window, char *word)
{
  char error_word[100];
  sprintf(error_word, "Error:\n\t\"%s\" does not exist!", word);
  /* Text */
  sfVector2f error_pos = {100, 100};
  sfText *error = sfText_create();
  sfText_setString(error, error_word);
  text_attribute(error, font, sfBlack, 20, error_pos);
  /* Title */
  title_text(title_word, "Delete");
  /* Display */
  while (sfRenderWindow_isOpen(window))
  {
    /* Process events */
    sfRenderWindow_clear(window, sfWhite);
    sfRenderWindow_drawSprite(window, background, NULL);
    sfRenderWindow_drawSprite(window, button_back, NULL);
    sfRenderWindow_drawText(window, back_bt, NULL);
    sfRenderWindow_drawSprite(window, button_home, NULL);
    sfRenderWindow_drawText(window, home_bt, NULL);
    sfRenderWindow_drawText(window, error, NULL);
    sfRenderWindow_drawText(window, title_word, NULL);
    sfRenderWindow_display(window);
    sfEvent event2;
    while (sfRenderWindow_pollEvent(window, &event2))
    {
      if (event2.type == sfEvtClosed)
        sfRenderWindow_close(window);
      if (event2.type == sfEvtMouseButtonPressed)
      {
        ifpos(home_pos, BW, BH)
        {
          sfText_destroy(error);
          return 0;
        }
        ifpos(back_pos, BW, BH)
        {
          sfText_destroy(error);
          return 1;
        }
      }
    }
  }
};

void delSuccess(sfRenderWindow *window, char *word)
{
  char del_word[100];
  sprintf(del_word, "Deleted \"%s\" from database!", word);
  /* Text */
  sfVector2f del_pos = {100, 100};
  sfText *del = sfText_create();
  sfText_setString(del, del_word);
  text_attribute(del, font, sfBlack, 20, del_pos);
  /* Title */
  title_text(title_word, "Delete");
  /* Button */
  /* Home */
  button_text(center_bt, center_bt_pos, BW, BH, "Home");
  /* Display */
  while (sfRenderWindow_isOpen(window))
  {
    /* Process events */
    sfRenderWindow_clear(window, sfWhite);
    sfRenderWindow_drawSprite(window, background, NULL);
    sfRenderWindow_drawSprite(window, button_center, NULL);
    sfRenderWindow_drawText(window, center_bt, NULL);
    sfRenderWindow_drawText(window, del, NULL);
    sfRenderWindow_drawText(window, title_word, NULL);
    sfRenderWindow_display(window);
    sfEvent event2;
    while (sfRenderWindow_pollEvent(window, &event2))
    {
      if (event2.type == sfEvtClosed)
        sfRenderWindow_close(window);
      if (event2.type == sfEvtMouseButtonPressed)
      {
        ifpos(center_bt_pos, BW, BH)
        {
          sfText_destroy(del);
          return;
        }
      }
    }
  }
};

void aboutScreen(sfRenderWindow *window)
{
  wchar_t about[2000] = L"English-Vietnamese Dictionary\nT??? ??i???n Anh Vi???t\n\nTh??nh vi??n:\n\tRed\n\tGreen\n\tBlue\n\tAlpha\n\nS??? d???ng th?? vi???n:\n\tCSFML 2.5.0 c???a Laurent Gomila d???a theo SFML\n\tB-tree 5.0.0 c???a Mark Willson\n\tSoundex\n\n\nT??? ??i???n g???m c??c ch???c n??ng:\n\t1. Tra t???\n\t2. Th??m t???\n\t3. X??a t???\n\nH??? tr??? nh???p :\n\tK?? t??? ASCII\n\tTab ????? ho??n thi???n t???\n\tControl+Num ????? d??ng t??? ???????c g???i ??";
  wchar_t word[MAX_PAGE][2000];
  int page;
  page = wordwrap(about, word);
  int current_page = 0;
  /* Button */
  /* Home */
  button_text(center_bt, center_bt_pos, BW, BH, "Home");
  /* Text */
  sfVector2f about_pos = {100, 120};
  sfText *about_word = sfText_create();
  text_attribute(about_word, font, sfBlack, 20, about_pos);
  /* Title */
  title_text(title_word, "About");
  while (sfRenderWindow_isOpen(window))
  {
    /* Process events */
    sfRenderWindow_clear(window, sfWhite);
    sfRenderWindow_drawSprite(window, background, NULL);
    if (page > 0 && current_page < page)
    {
      sfRenderWindow_drawSprite(window, button_next, NULL);
      sfRenderWindow_drawText(window, next_bt, NULL);
    }
    if (page > 0 && current_page > 0)
    {
      sfRenderWindow_drawSprite(window, button_prev, NULL);
      sfRenderWindow_drawText(window, prev_bt, NULL);
    }
    sfRenderWindow_drawSprite(window, button_center, NULL);
    sfRenderWindow_drawText(window, center_bt, NULL);
    sfText_setUnicodeString(about_word, word[current_page]);
    sfRenderWindow_drawText(window, about_word, NULL);
    sfRenderWindow_drawText(window, title_word, NULL);
    sfRenderWindow_display(window);
    sfEvent event2;
    while (sfRenderWindow_pollEvent(window, &event2))
    {
      if (event2.type == sfEvtClosed)
        sfRenderWindow_close(window);
      if (event2.type == sfEvtMouseButtonPressed)
      {
        ifpos(center_bt_pos, BW, BH)
        {
          sfText_destroy(about_word);
          return;
        }
        ifpos(next_pos, BW, BH) if (current_page < page)
            current_page++;
        ifpos(prev_pos, BW, BH) if (current_page > 0)
            current_page--;
      }
    }
  }
};

int textureInit()/* create texture and text */
{
  button = sfTexture_createFromFile(BUTTON, NULL);
  long_button = sfTexture_createFromFile(LONGBUTTON, NULL);
  bg = sfTexture_createFromFile(BACKGROUND, NULL);
  font = sfFont_createFromFile(FONT);
  if(!button || !long_button || !bg || !font)
    return 0;
  button_home = sfSprite_create();
  button_create(button_home, home_pos.x, home_pos.y, button);
  button_back = sfSprite_create();
  button_create(button_back, back_pos.x, back_pos.y, button);
  button_next = sfSprite_create();
  button_create(button_next, next_pos.x, next_pos.y, button);
  button_prev = sfSprite_create();
  button_create(button_prev, prev_pos.x, prev_pos.y, button);
  button_center = sfSprite_create();
  button_create(button_center, center_bt_pos.x, center_bt_pos.y, button);
  background = sfSprite_create();
  sfSprite_setTexture(background, bg, sfTrue);
  text_word = sfText_create();
  text_attribute(text_word, font, sfBlack, 15, text_pos);
  back_bt = sfText_create();
  button_text(back_bt, back_pos, BW, BH, "Back");
  home_bt = sfText_create();
  button_text(home_bt, home_pos, BW, BH, "Home");
  next_bt = sfText_create();
  button_text(next_bt, next_pos, BW, BH, "Next");
  prev_bt = sfText_create();
  button_text(prev_bt, prev_pos, BW, BH, "Previous");
  center_bt = sfText_create();
  title_word = sfText_create();
  return 1;
}

sfVector2f button_create(sfSprite *bt, float x, float y, sfTexture *texture)/* create buttom */
{
  sfSprite_setTexture(bt, texture, sfTrue);
  sfVector2f bt_pos = {x, y};
  sfSprite_setPosition(bt, bt_pos);
  return bt_pos;
}

void button_text(sfText *text, sfVector2f bt_pos, float bt_width, float bt_height, char *string)
{
  sfText_setString(text, string);
  sfText_setFont(text, font);
  sfText_setCharacterSize(text, 20);
  sfText_setColor(text, sfWhite);
  sfText_setOutlineThickness(text, 1.5);
  sfText_setOutlineColor(text, sfBlack);
  sfFloatRect text_rect = sfText_getGlobalBounds(text);
  sfVector2f text_pos = {bt_pos.x + bt_width / 2 - text_rect.width / 2, bt_pos.y + bt_height / 2 - text_rect.height / 2};
  sfText_setPosition(text, text_pos);
}

void title_text(sfText *text, char *string)
{
  sfText_setString(text, string);
  sfText_setFont(text, font);
  sfText_setCharacterSize(text, 30);
  sfText_setColor(text, sfBlack);
  sfText_setStyle(text, sfTextBold);
  sfFloatRect text_rect = sfText_getGlobalBounds(text);
  sfVector2f text_pos = {WW / 2 - text_rect.width / 2, (BH + back_pos.y) / 2 - text_rect.height / 2};
  sfText_setPosition(text, text_pos);
};

void text_attribute(sfText *text, sfFont *font, sfColor color, int size, sfVector2f pos)
{
  sfText_setFont(text, font);
  sfText_setCharacterSize(text, size);
  sfText_setColor(text, color);
  sfText_setPosition(text, pos);
}

void destroy_globalTexture() /* Clean global resources */
{
  sfTexture_destroy(button);
  sfTexture_destroy(long_button);
  sfTexture_destroy(bg);
  sfFont_destroy(font);
  sfSprite_destroy(button_home);
  sfSprite_destroy(button_back);
  sfSprite_destroy(button_next);
  sfSprite_destroy(button_prev);
  sfSprite_destroy(button_center);
  sfSprite_destroy(background);
  sfText_destroy(text_word);
  sfText_destroy(back_bt);
  sfText_destroy(home_bt);
  sfText_destroy(next_bt);
  sfText_destroy(prev_bt);
  sfText_destroy(center_bt);
  sfText_destroy(title_word);
}

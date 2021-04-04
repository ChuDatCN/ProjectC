#include<stdlib.h>
#include<string.h>
#include<wchar.h>

#define LINE_CHAR 60
#define LINE 15
#define MAX_PAGE 10

int wordwrap(wchar_t *word, wchar_t word_page[MAX_PAGE][2000]){/* auto wrap word and new page */
  int count=0;
  for(int i=0;i<wcslen(word);i++){/* auto wrap word */
    if(word[i]!='\n'){
      count++;
      if(count==LINE_CHAR+1){
        int c_count=0;
        while(word[i]!=' '){
          c_count++;
          if(c_count > 20){
            i+=c_count-1;
            memcpy(word+i+1, word+i, wcslen(word)-i+2);
            break;
          }
          i--;
        }
        word[i] = '\n';
        count=0;
      }
    }
    else
      count=0;
  }
  int page = 0;/* Page break */
  count=0;
  int cut_pos[MAX_PAGE];
  cut_pos[0] = 0;
  wcscpy(word_page[page], word);
  for(int i=0;i<wcslen(word);i++){
    if(word[i]=='\n')
      count++;
    if(count==LINE){
      count=0;
      page++;
      wcscpy(word_page[page], word+i+1);
      word_page[page-1][i-cut_pos[page-1]] = 0;
      cut_pos[page] = i;
    }
  }
  if(!wcslen(word_page[page]))
    page--;
  return page;
}

int add_wordwrap(char *word){/* auto wrap word only */
  int newline_check = 0;
  int count=0;
  for(int i=0;i<strlen(word);i++){
    if(word[i]!='\n'){
      count++;
      if(count==LINE_CHAR+1){
        int c_count=0;
        while(word[i]!=' '){
          c_count++;
          if(c_count > 20){
            i+=c_count-1;
            word[i+2] = '\0';
            word[i+1] = word[i];
            word[i] = '\n';
            break;
          }
          i--;
        }
        word[i] = '\n';
        newline_check = 1;
        count=0;
      }
    }
    else
      count=0;
  }
  return newline_check;
}
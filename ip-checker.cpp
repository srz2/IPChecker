#include <stdio.h>
#include <string.h>
#include <cstdlib>

#include <iostream>
#include <fstream>
using namespace std;

#define PATH_DEFAULT "echo $HOME/.ip-check"
#define FILE_LAST_IP ".lastip"
#define FILE_CONTACT_LIST ".contacts"

int main(int argc, char ** argv)
{
  FILE * in = NULL;
  char pathDefault[1024];
  char pathLastIP[1024];
  char pathContacts[1024];

  //Construct Program hidden directory
  in = popen(PATH_DEFAULT, "r");
  if(in == NULL)
  {
    printf("Unable to read default path %s\n", PATH_DEFAULT);
    return 1;
  }
  fscanf(in, "%s", pathDefault);
  pclose(in);
  in = NULL;

  //Create Hidden Directory
  char t_cmd[1024];
  sprintf(t_cmd, "mkdir -p %s", pathDefault);
  system(t_cmd);
  printf("%s\n", t_cmd);

  //Create last ip file
  sprintf(pathLastIP, "%s/%s", pathDefault, FILE_LAST_IP);
  memset(t_cmd, '\0', 1024);
  strcat(t_cmd, "touch ");
  strcat(t_cmd, pathLastIP);
  system(t_cmd);

  //Create contacts list
  sprintf(pathContacts, "%s/%s", pathDefault, FILE_CONTACT_LIST);
  memset(t_cmd, '\0', 1024);
  strcat(t_cmd, "touch ");
  strcat(t_cmd, pathContacts);
  system(t_cmd);

  //Query and read in public IP Address
  in = popen("curl -s https://api.ipify.org | cat", "r");
  if(in == NULL)
  {
    printf("Unable to read api command results\n");
    return 2;
  }

  char ipPublic[32];
  fscanf(in, "%s", ipPublic);
  pclose(in);
  in = NULL;

  //Query and read in last IP Address sent with program
  in = fopen(pathLastIP, "r");
  if(in == NULL)
  {
    printf("Unable to read last IP Address\n");
    return 3;
  }

  char ipLast[25];
  fscanf(in, "%s", ipLast);
  fclose(in);
  in = NULL;

  //Check if file exists
  if(strstr(ipLast, "No such file or directory") != NULL)
  {
    printf("File is not found\n");
    return 3;
  }

  // printf("Last IP:    %s\n", ipLast);
  // printf("Current IP: %s\n", ipPublic);
  if(strcmp(ipLast, ipPublic) == 0)
  {
    printf("Current Public IP hasn't changed (%s).\n", ipLast);
    return 0;
  }
  else
  {
    //Replace last ip with current public ip
    char cmd_replace[64];
    sprintf(cmd_replace, "echo %s > %s", ipPublic, pathLastIP);
    system(cmd_replace);

    //Create Message
    char msg[256];
    sprintf(msg, "The server's IP has changed again. The new address is %s:32400/web/index.html", ipPublic);
    
    //Extract and execute command for each contact
    ifstream inFile(pathContacts);
    if(!inFile)
    {
      printf("[ERROR] Unable to open contact list!\n");
      return 4;
    }
    string name = "", number = "";
    int count = 0;
    while(inFile >> name >> number)
    {      
      //Create Command
      char cmd[1024];
      sprintf(cmd, "curl -X POST http://textbelt.com/text -d number=%s -d \"message=%s\"", number.c_str(), msg);

      //Execute Command
      system(cmd);

      count++;
    }
    inFile.close();

    cout << "Attempted to notify " << count << " people!" << endl;
  }
}

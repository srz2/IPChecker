#include <stdio.h>
#include <string.h>
#include <cstdlib>

#include <iostream>
#include <fstream>
using namespace std;

#define PATH_LAST_IP "ls $HOME/.ip-check/.lastip"
#define PATH_CONTACT_LIST "ls $HOME/.ip-check/.contacts"

int main(int argc, char ** argv)
{
  FILE * in = NULL;
  char pathLastIP[1024];
  char pathContacts[1024];

  if(argc == 2)
  {
    //Construct last IP Path from Arguments
    size_t len = strlen(argv[1]);
    if(len > 1024)
    {
      printf("Argment path length is larger than %d - %lu", 1024, len);
      return 1;
    }

    strcpy(pathLastIP, argv[1]);
  }
  else
  {
    //Construct last IP Path from Defaults
    in = popen(PATH_LAST_IP, "r");
    if(in == NULL)
    {
      printf("Unable to read default path %s\n", PATH_LAST_IP);
      return 1;
    }
    fscanf(in, "%s", pathLastIP);
    pclose(in);
    in = NULL;

    //Construct contacts path from Defaults
    in = popen(PATH_CONTACT_LIST, "r");
    if(in == NULL)
    {
      printf("Unable to read default path %s\n", PATH_CONTACT_LIST);
      return 1;
    }
    fscanf(in, "%s", pathContacts);
    pclose(in);
    in = NULL;
  }

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
    printf("Current public IP is the same as before (%s), doing nothing...\n", ipLast);
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
    while(inFile >> name >> number)
    {
      //Create Command
      char cmd[1024];
      sprintf(cmd, "curl -X POST http://textbelt.com/text -d number=%s -d \"message=%s\"", number.c_str(), msg);

      //Execute Command
      system(cmd);
    }
    inFile.close();
  }
}

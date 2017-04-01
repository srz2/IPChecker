#include <stdio.h>
#include <string.h>
#include <cstdlib>

#include <iostream>
#include <fstream>
#include <vector>
using namespace std;

#define PATH_DEFAULT "echo $HOME/.ip-check"
#define FILE_LAST_IP ".lastip"
#define FILE_CONTACT_LIST ".contacts"

class Contact
{
private:
	string name;
	string number;
public:
	Contact(string name, string number)
	{
		for(int c = 0;  c < name.length(); c++)
		{
			name[c] = tolower(name[c]);
		}
		name[0] = toupper(name[0]);

		this->name = name;
		this->number = number;
	}

	string getName()
	{
		return this->name;
	}

	string getNumber()
	{
		return this->number;
	}

	string toString()
	{
		return this->getName() + " " + this->getNumber(); 
	}

	bool operator ==(Contact & right)
	{
		if(this->name == right.getName())
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	bool operator ==(const string & right)
	{
		if(this->name == right)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
};

vector<Contact> contacts;

void load(string path);
void save(string path);

int main(int argc, char ** argv)
{
	if(argc == 1)
	{
		cout << "Run Program:    ip-check -r" << endl;
		cout << "List Contacts:  ip-check -l" << endl;
		cout << "Add Contact:    ip-check -a [NAME] [NUMBER]" << endl;
		cout << "Delete Contact: ip-check -d [NAME] " << endl;
		return 0;
	}

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

//Load Contacts
load(pathContacts);

//Evaluate program arguments
  if(argc >= 2 && argc <= 4)
  {
    char * arg_cmd  = argv[1];
    char * arg_name = argv[2];

    if(strcmp(arg_cmd, "-a") == 0)
    {
      char * arg_number = argv[3];

	//Check if contact is in list
	int index = -1;
	bool exists = false;
	for(vector<Contact>::iterator it = contacts.begin(); it != contacts.end(); it++)
	{
		index++;
		Contact c = *it;
		if(c == arg_name)
		{
			exists = true;
			break;
		}
	}

	//If it exists, add to contact list and save
	if(!exists)
	{
		Contact c = Contact(arg_name, arg_number);
		contacts.push_back(c);
		save(pathContacts);
		cout << "Added " << c.toString() << endl;	
	}
	else
	{
		Contact c = contacts[index];
		cout << c.toString() << " already in contact list" << endl;
	}
	return 0;
    }
    else if(strcmp(arg_cmd, "-d") == 0)
    {
	//Check if contact is in list
	int index = -1;
	bool exists = false;
	for(vector<Contact>::iterator it = contacts.begin(); it != contacts.end(); it++)
	{
		index++;
		Contact c = *it;
		if(c == arg_name)
		{
			exists = true;
			break;
		}
	}

	if(exists)
	{
		Contact c = contacts[index];
		contacts[index] = contacts[contacts.size() - 1];
		contacts[contacts.size() - 1] = c;
		contacts.pop_back();
		cout << "Deleted " << c.toString() << endl;
		save(pathContacts);
		return 0;
	}
	else
	{
		cout << arg_name << " Doesn't Exist!" << endl;
	}
	
	return 0;
    }
   else if(strcmp(arg_cmd, "-l") == 0)
    {
	for(vector<Contact>::iterator it = contacts.begin(); it != contacts.end(); it++)
	{
		Contact c = *it;
		cout << c.toString() << endl;
	}
	return 0;	
    }
   else if(strcmp(arg_cmd, "-r") == 0)
    {

    }
    else
    {
	cout << "Unknown argment " << arg_cmd << endl;
    }
  }
  else
  {
	cout << "[WARN]: Shouldn't get here..." << endl;
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
    const char * KEY_TEXTBELT = getenv("KEY_TEXTBELT");
    while(inFile >> name >> number)
    {      
      //Create Command
      char cmd[1024];
      sprintf(cmd, "curl -X POST https://textbelt.com/text -d key=\"%s\" --data-urlencode phone=\"%s\" --data-urlencode message=\"%s\"", KEY_TEXTBELT, number.c_str(), msg);

      //Execute Command
      system(cmd);

      count++;
    }
    inFile.close();

    cout << "Attempted to notify " << count << " people!" << endl;
  }
}

void load(string path)
{
	contacts.clear();

	ifstream inFile(path.c_str());
	if(!inFile)
	{
		cout << "Unable to load contacts at " << path << endl;
		exit(1);
	}

	string name = "";
	string number = "";
	while(inFile >> name >> number)
	{
		Contact c = Contact(name, number);
		contacts.push_back(c);
	}
	inFile.close();

	cout << "Loaded " << contacts.size() << " Contacts" << endl;
}

void save(string path)
{
	ofstream outFile(path.c_str());

	for(vector<Contact>::iterator it = contacts.begin(); it != contacts.end(); it++)
	{
		Contact c = *it;
		outFile << c.toString() << endl;
	}
	outFile.close();
}


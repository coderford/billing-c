/*CSE101 Mini Project: Shopping Mall Billing System
	By Aditya Kumar, 
	Section K1615, Roll B48, Reg 11603774 */
#include <stdio.h>
#include <conio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <time.h>
#define defpass "12345#"
#define defuser "admin"
#define psize 20
#define nsize 30
#define mall "               Big Bazaar"
#define addr "     Lovely Professional University"
#define initproid 13210000
#define initbilid 20160000
#define proidheader "132"			//assuming that no product's name contains this
#define tax 0.14
#define cryptkey 6
/* files:
products.dat -- contains product data
user.dat -- contains users and their passwords
bill(n).txt -- the bill file that will be printed 
meta.dat -- contains meta-info about products... like no, last id etc
We'll assume that the width of bill permits 40 characters */

typedef struct{
	int day, month, year;
}date;

typedef struct{				//Product structure
	char name[nsize];
	int id;
	float price;
	char unit[10];
	//int sold;
} product;

typedef struct{				//bill structure
	char filename[nsize];
	char customer[nsize];
	long id;
	float amount;
	date purchase;
} bill;

typedef struct{				//user structure
	char passwd[psize];
	char uname[nsize];
} user;

char pass[psize];			//global variables
char username[nsize];
/*---Prototypes---*/
void decrypt(char str[]);		//Decrypts encrypted pass string from the passwd file into the real password	
void encrypt(char str[]);		//Encrypts the changed password before it's stored into the passwd file
int login();					//Returns 1 if login is successful, otherwise 0
int getpro(product *, char *);	//finds a product with name or id
int addnew();					//adds a new product record in the products.dat file
int billgen();					//generates bill
int deldata();					//deletes product records
int showdata();					//displays data
int search();					//searches for a product
int isempty(char []);			//checks whether a file is empty or non-existent
void chpass(); 					//change password
void adduser();					//add a new user
void deluser();					//remove a user
int scanuser(char [], char [], char []);			//gets password from file
void getpass(char [], char[]);	//gets password from user
void strlow(char []);			//converts string to lower case 
int fdisp(char []);				//simply displays a file
int usermgmt();					//User management
int searchbill();				//search bills
	
int main(){
	int choice;
	if(!login()){
		printf("Login failed!");
		getch();
		return 0;
	}
	do{
		system("cls");
		printf("                        SHOPPING MALL BILLING SYSTEM\n");
		printf("--------------------------------------------------------------------------------\n");
		printf("What will you do?\n\n");
		printf("1. Add product\n");
		printf("2. Generate bill\n");
		printf("3. Display product details\n");
		printf("4. Search for a product\n");
		printf("5. Delete product data\n");
		printf("6. Display a bill\n");
		printf("7. Search for bill\n");
		if(strcmp(username, "admin")==0)
		printf("8. User management\n");
		else 
		printf("8. Change password\n");
		printf("9. Exit\n");
		printf("\nEnter your choice: ");
		scanf("%d", &choice);
		switch(choice){
			case 1: addnew(); break;
			case 2: billgen(); break;
			case 3: showdata(); break;
			case 4: search(); break;
			case 5: deldata(); break;
			case 6: dispbill(); break;
			case 7: searchbill(); break;
			case 8: if(strcmp(username, "admin")==0)
					usermgmt(); 
					else chpass(); break;
			case 9: return 0; 
			default: printf("Wrong choice!"); getch();
		}
	} while(choice!=9);	
	return 0;
}

int addnew(){
	product tmp, tmp2;
	FILE *products, *meta;
	char choice='Y', ch;
	int count, lastid, found;
	isempty("products.dat");
	if(isempty("meta.dat")){
		meta=fopen("meta.dat", "w");
		fprintf(meta, "0 %ld", initproid);
		fclose(meta);
	}
	while(toupper(choice)=='Y'){												
		system("cls");
		printf("NEW PRODUCT\n");
		printf("Name: ");
		fflush(stdin);
		gets(tmp.name);
		if(found=getpro(&tmp2, tmp.name)){
			printf("\nThis product is already in the database! Continue? (Y/N)");
			scanf("%c", &choice);
			continue;
		}
		products=fopen("products.dat", "a");	
		meta=fopen("meta.dat", "r");
		fscanf(meta, "%d %d", &count, &lastid);
		fclose(meta);
		meta=fopen("meta.dat", "w");
		printf("Price: ");
		scanf("%f", &tmp.price); 
		printf("Unit: ");
		scanf("%s", tmp.unit);
		tmp.id=lastid+1;
		fwrite(&tmp, sizeof(product), 1, products);
		count++; lastid++;
		fprintf(meta, "%d %d", count, lastid);
		printf("Record appended successfully!\n");
		printf("Enter more details? (Y/N) ");
		fflush(stdin);
		scanf("%c", &choice);
		choice=toupper(choice);
		fclose(products);
		fclose(meta);
	}
	return 0;
}

int showdata(){
	system("cls");
	int found, empty;
	char ch, name_id[nsize];
	product tmp;
	FILE *products;
	if(isempty("products.dat")){
		printf("No data yet!");
		getch();
		return 0;
	}	
	products=fopen("products.dat", "r");
	printf("Enter product name or ID: ");
	fflush(stdin);
	gets(name_id);
	found=getpro(&tmp, name_id);
	if(found){
		system("cls");
		printf("Details of product:\n\n");
		printf("Name: %s\n", tmp.name);
		printf("Price: Rs %.2f per %s\n", tmp.price, tmp.unit );
		printf("ID: %d", tmp.id);
	}
	else printf("No such product found!");
	fclose(products);
	getch();
	return 0;
}

int billgen(){
	system("cls");
	FILE *products;
	if(isempty("products.dat")){
		printf("No data yet!");
		getch();
		return 0;
	}	
	products=fopen("products.dat", "r");
	FILE *bilfile, *meta, *bildata;
	bill tmpbill;
	product tmp;
	char choice1, choice2, name_id[nsize], billname[20]="bills\\bill", ch, billno[10], paymode[20];
	int billcount, itemcount=0, found, qty;
	float total=0, final;
	long lasttxn;
	time_t raw;
	struct tm local;
	if(!(meta=fopen("bills\\bmeta.dat", "r")))
		meta=fopen("bills\\bmeta.dat", "w");
	fclose(meta);
	meta=fopen("bills\\bmeta.dat", "r");
	if((ch=fgetc(meta))==EOF){
		fclose(meta);
		meta=fopen("bills\\bmeta.dat", "w");
		fprintf(meta, "0 %ld", initbilid);
		fclose(meta);
		meta=fopen("bills\\bmeta.dat", "r");
	}
	else rewind(meta);
	fscanf(meta, "%d %ld", &billcount, &lasttxn);
	sprintf(billno, "%d", billcount+1);
	strcat(billname, billno);
	strcat(billname, ".txt");
	strcpy(tmpbill.filename, billname);	
	time(&raw);
	local=*(localtime(&raw));
	tmpbill.purchase.day=local.tm_mday;				//----------------
	tmpbill.purchase.month=local.tm_mon+1;				//storing date from local time to tmpbill's purchase date
	tmpbill.purchase.year=local.tm_year+1900;		//----------------
	bilfile=fopen(billname, "w");
	fprintf(bilfile, "########################################\n\n\n");
	fprintf(bilfile, "%s\n%s\n", mall, addr);
	fprintf(bilfile, "----------------------------------------\n");
	fprintf(bilfile, "Staff: %-10s  Invoice no:%10ld\n", username, ++lasttxn);
	tmpbill.id=lasttxn;
	fprintf(bilfile, "Date: %d/%d/%d          Time: %02d:%02d:%02d\n", local.tm_mday, local.tm_mon+1, local.tm_year+1900,
													  local.tm_hour, local.tm_min, local.tm_sec);
	printf("Enter customer's name: ");
	fflush(stdin);
	gets(tmpbill.customer);
	char mr_mrs[nsize]="Mrs/Mr. ";
	fprintf(bilfile, "Invoice for: %27s\n", strcat(mr_mrs, tmpbill.customer));												  
	fprintf(bilfile, "----------------------------------------\n");
	fprintf(bilfile, "Item Name   Price/Unit    Qty.     Value\n\n");
	fprintf(bilfile, "----------------------------------------\n");
	system("cls");
	
	do{
		system("cls");
		printf("Details of purchase:\n\n");
		printf("Enter name or ID of product: ");
		fflush(stdin);
		gets(name_id);
		found=getpro(&tmp, name_id);
		if(!found){
			printf("No such product found! Add new? (Y/N)");
			scanf("%c", &choice1);
			if(toupper(choice1)=='Y')
			addnew();
			choice2='Y';
			continue;	
		}
		printf("Enter quantity: ");
		scanf("%d", &qty);
		fprintf(bilfile, "%-12s", tmp.name);
		if(strlen(tmp.name)>12)
		fprintf(bilfile, "\n            ");
		fprintf(bilfile, "%-14.2f", tmp.price);
		fprintf(bilfile, "%-6d",qty);
		fprintf(bilfile, "%8.2f\n", qty*tmp.price);
		itemcount+=qty;
		total+=qty*tmp.price;
		printf("Enter more products? (Y/N)");
		fflush(stdin);
		scanf("%c", &choice2);
	} while (toupper(choice2)=='Y');
	printf("Enter payment method: ");
	fflush(stdin);
	gets(paymode);
	fprintf(bilfile, "----------------------------------------\n");
	fprintf(bilfile, "Item Count: %3d%15s%10.2f\n", itemcount,"Total:", total);
	fprintf(bilfile, "----------------------------------------\n");
	fprintf(bilfile, "VAT    %2.0f%c\n",(float)tax*100, '%');
	fprintf(bilfile, "TAX%37.2f\n", total*tax);
	fprintf(bilfile, "----------------------------------------\n");
	fprintf(bilfile, "Amount Due%30.2f\n", (final=total+total*tax)); 			
	tmpbill.amount=final;			//store amount in billdata
	fprintf(bilfile, "----------------------------------------\n");
	fprintf(bilfile, "Payment Mode\n");
	fprintf(bilfile, "%-20s%20.2f\n", paymode, final);
	fprintf(bilfile, "----------------------------------------\n");
	fprintf(bilfile, "\n        ********THANK YOU*******\n");
	fprintf(bilfile, "           Please visit again\n");
	fprintf(bilfile, "\n\n########################################");
	billcount++;
	fclose(meta);
	meta=fopen("bills\\bmeta.dat", "w");
	fprintf(meta, "%d %ld", billcount, lasttxn);
	fclose(meta);
	fclose(bilfile);
	printf("Display generated bill? (Y/N) ");
	scanf("%c", &choice1);
	if(toupper(choice1)=='Y'){
		system("cls");
		fdisp(billname);
		getch();
	}
	bildata=fopen("bills\\billdata.dat", "a");
	fwrite(&tmpbill, sizeof(bill), 1, bildata);
	fclose(bildata);
	return 0;
}

int deldata(){			
	product key, tmp;
	FILE *old, *current, *meta;
	char name_id[nsize], choice='Y';
	int found, procount;
	long lastid;
	system("cls");
	if(isempty("products.dat")){
		printf("Nothing to delete!");
		getch();
		return 0;
	}
	do{
		system("cls");
		//1. Search for the product to delete
		printf("Enter product name or ID: ");
		fflush(stdin);
		gets(name_id);
		found=getpro(&key, name_id);
		if(!found){
			printf("No such product found! Continue? (Y/N)");
			scanf("%c", &choice);
			choice=toupper(choice);
			continue;
		}
		//2. copy from current to old 
		old=fopen("old.dat", "w");
		current=fopen("products.dat", "r");
		while(!feof(current)){
			if(fread(&tmp, sizeof(product), 1, current)==0) break;
			fwrite(&tmp, sizeof(product), 1, old);	
		}
		fclose(old);
		fclose(current);
		old=fopen("old.dat", "r");
		current=fopen("products.dat", "w");
		meta=fopen("meta.dat", "r");
		fscanf(meta, "%d %ld", &procount, &lastid);
		fclose(meta);
		meta=fopen("meta.dat", "w");
		//3. copy back from old to current, not including product to be deleted
		while(!feof(old)){
			if(fread(&tmp, sizeof(product), 1, old)==0) break;
			if(tmp.id!=key.id)
			fwrite(&tmp, sizeof(product), 1, current);
		}
		fprintf(meta, "%d %ld", procount-1, lastid);
		fclose(old); fclose(current); fclose(meta);
		remove("old.dat");
		printf("Deleted Successfully!\n");
		printf("Delete more? (Y/N) ");
		scanf("%c", &choice);	
	} while(toupper(choice)=='Y');
}

int search(){					//lists all matching products
	FILE *products;
	product tmp;
	char name[nsize], tmpname[nsize];
	int empty=isempty("products.dat"), found=0;		//check if data is available
	system("cls");
	if(empty){
		printf("No data yet!");
		getch();
		return 0;
	}
	printf("Type ALL to list all products\n\n");
	printf("Enter product name: ");
	fflush(stdin);
	gets(name);
	products=fopen("products.dat", "r");
	if(strcmp(name, "ALL")==0){
		while(!feof(products)){
			if(fread(&tmp, sizeof(product), 1, products)==0) break;
			printf("Name:  %-30s    ID: %ld\n", tmp.name, tmp.id);
		}
		getch();
		fclose(products);
		return 1;
	}
	while(!feof(products)){								//feof doesn't return 1 until EOF is read
		if(fread(&tmp, sizeof(product), 1, products)==0) //so fread fails just before EOF and returns 0	
			break;
		strcpy(tmpname, tmp.name);						//if break is not used, last item will be_
		strlow(tmpname); strlow(name);					//printed twice
		if(strstr(tmpname, name)!=NULL){
			if(!found){
				printf("Following matches found:\n\n");
				found=1;
			}
			printf("Name:  %-30s    ID: %ld\n", tmp.name, tmp.id);
		}
	}
	if(!found){
		printf("No matches found!\n");
		getch();
		return 0;
	}
	getch();
	fclose(products);
	return 1;	
}

int getpro(product *key, char name_id[]){
	int mode=0; //mode -- 0 means search by id, 1 means search by name
	long id;	
	FILE *products;
	products=fopen("products.dat", "r");
	if(strstr(name_id, proidheader)==NULL)
		mode=1;
	switch(mode){
		case 0: sscanf(name_id, "%ld", &id);
				while(!feof(products)){
					fread(key, sizeof(product), 1, products);
					if(key->id==id){
						fclose(products);
						return 1;				
					}	
				}
		case 1:	while(!feof(products)){
					fread(key, sizeof(product), 1, products);
					if(stricmp(key->name, name_id)==0){
						fclose(products);
						return 1;				
					}
				}			
	}	
	fclose(products);
	return 0;		
}

int searchbill(){					//lists all matching bills
	FILE *bildata;
	bill tmp;
	char key[nsize], tmpname[nsize];
	int empty=isempty("bills\\billdata.dat"), found=0, mode=0;	//mode: 1--by customer, else date	
	int day, month, year;	
	system("cls");
	if(empty){						//check if data is available
		printf("No bills yet!");
		getch();
		return 0;
	}
	printf("Type ALL to get all bills\n\n");
	printf("Enter customer name or date of purchase: ");
	fflush(stdin);
	gets(key);
	if(strchr(key, '/')==NULL)				//assuming that a customer's name does not contain /
	mode=1;
	bildata=fopen("bills\\billdata.dat", "r");
	int i=1;
	if(strcmp(key, "ALL")==0){
		while((fread(&tmp, sizeof(bill), 1, bildata))>0){
			printf("%02d. Date: %d/%d/%d		Customer: %-8s  	Amount: Rs. %-5.2f\n    TXNID: %ld  		File: %s\n\n", 
			i, tmp.purchase.day,tmp.purchase.month, tmp.purchase.year, tmp.customer, tmp.amount, tmp.id, tmp.filename);
			i++;
		}
		getch();
		fclose(bildata);
		return 1;
	}
	switch(mode){
		case 0: sscanf(key, "%d/%d/%d", &day, &month, &year);
				while(fread(&tmp, sizeof(bill), 1, bildata)>0){							
					if(tmp.purchase.day==day&&tmp.purchase.month==month&&tmp.purchase.year==year){
						if(!found){
							printf("Following matches found:\n\n");
							found=1;
						}
						printf("%02d. Date: %d/%d/%d		Customer: %-8s  	Amount: Rs. %-5.2f\n    TXNID: %ld  		File: %s\n\n", 
						i, tmp.purchase.day,tmp.purchase.month, tmp.purchase.year, tmp.customer, tmp.amount, tmp.id, tmp.filename);
						i++;
					}
				} 
				break;
		case 1:	while(fread(&tmp, sizeof(bill), 1, bildata)>0){		
				strcpy(tmpname, tmp.customer);					
				strlow(tmpname); strlow(key);					
					if(strstr(tmpname, key)!=NULL){
						if(!found){
							printf("Following matches found:\n\n");
							found=1;
						}
						printf("%02d. Date: %d/%d/%d		Customer: %-8s  	Amount: Rs. %-5.2f\n    TXNID: %ld  		File: %s\n\n", 
						i, tmp.purchase.day,tmp.purchase.month, tmp.purchase.year, tmp.customer, tmp.amount, tmp.id, tmp.filename);
						i++;
					}
				}		
	}
	if(!found){
		printf("No matches found!\n");
		getch();
		return 0;
	}
	getch();
	fclose(bildata);
	return 1;	
}

int dispbill(){
	long txn;
	int found=0;
	bill tmp;
	FILE *bildata;
	if(isempty("bills\\billdata.dat")){
		printf("No bill data!");
		getch();
		return 0;
	}
	printf("Enter transaction id: ");
	scanf("%ld", &txn);
	bildata=fopen("bills\\billdata.dat", "r");
	while(fread(&tmp, sizeof(bill), 1, bildata)>0){
		if(tmp.id==txn){
			found=1;
			break;
		}
	}
	fclose(bildata);
	if(!found){
		printf("No such transaction!");
		getch();
		return 0;
	}
	system("cls");
	fdisp(tmp.filename);
	getch();
	return 0;
}

void getpass(char pwd[], char prompt[]){	//get passwords from user
	system("cls");
	char ch; 					//getting pass char by char
	int i=0, j;					//i for each char press, j for printing the astericks 
	printf("%s", prompt);  		//start with displaying the prompt
	while((ch=getch())!='\r'){	//getch() seems to take carriage return as end
		j=0;	
		system("cls");			//the prompt has to be updated every time something is typed
		printf("%s", prompt);	
		if(ch=='\b'){			//if user presses backspace, don't store
			if(i>0)				//if entered pass isn't empty remove last char from pass string
			i--;	
		}
		else{
			pwd[i]=ch;			
			i++;
		}
		while(j<i){				//printing the astericks
			putchar('*');
			j++;
		}
	}				
	pwd[i]='\0';				//completing the string
	system("cls");
}

int scanuser(char loginname[], char username[], char pass[]){				//scan user and password from the user.dat file
	if(isempty("user.dat")){
		if(strcmp(loginname, "admin")==0){
			strcpy(pass, defpass);
			strcpy(username, "admin");
			return 1;
		}
		return -1;						//-1  means no users added
	}						
	user u;			
	FILE *ufile;
	ufile=fopen("user.dat", "r");
	while(fread(&u, sizeof(user), 1, ufile)>0){
		if(strcmp(u.uname, loginname)==0){
			strcpy(username, u.uname);
			strcpy(pass, u.passwd);
			decrypt(pass);
			fclose(ufile);
			return 1;					//1 means user found
		}	
	}

	fclose(ufile);
	return 0;							//0 means user not found
}

int login(){	
	char uname[nsize];		
	char cont='Y', usrpwd[psize];
	int  usrfound;
	while(toupper(cont)!='N'){
		system("cls");
		printf("Enter username: ");
		fflush(stdin);
		gets(uname);
		usrfound=scanuser(uname, username, pass);
		if(usrfound==-1){
			printf("No users yet -- login with admin account and default password\n");
			printf("Coninue? (Y/N)");
			fflush(stdin);
			scanf("%c", &cont);
			continue;
		}
		else if(usrfound==0){
			printf("No such user! \nContinue? (Y/N)");
			fflush(stdin);
			scanf("%c", &cont);
			continue;
		}
		system("cls");
		getpass(usrpwd, "Enter password: ");	//get pass from user and compare
		if(strcmp(pass, usrpwd)==0)
		return 1;	
		else {									//if not successful, ask to exit
			printf("Wrong password! Continue? (Y/N) ");
			fflush(stdin);
			scanf("%c", &cont);
			if(toupper(cont)=='N'){
				system("cls");
				return 0;
			}		
		}
	}
}

int usermgmt(){
	user u;
	char pwd[psize];
	int choice;
	FILE *ufile;
	if(isempty("user.dat")){				//admin will not be able to login if other users are created first
		strcpy(u.uname, "admin");
		strcpy(u.passwd, pass);
		encrypt(u.passwd);
		ufile=fopen("user.dat", "w");
		fwrite(&u, sizeof(user), 1, ufile);
		fclose(ufile);
	}
	do{
		system("cls");
		printf("What will you do?\n");
		printf("1. Change your password\n");
		printf("2. Add a user\n");
		printf("3. Delete a user\n");
		printf("4. Back\n");
		printf("Enter your choice: ");
		scanf("%d", &choice);
		switch(choice){
			case 1: chpass(); break;
			case 2: adduser(); break;
			case 3: deluser(); break;
			case 4: break;
			default: printf("Wrong Choice!\n");
		}
	}while(choice!=4);
}

void chpass(){
	char cur[psize];
	FILE *ufile, *old;
	user updated, tmp;
	system("cls");
	getpass(cur, "Enter current password: ");
	if(strcmp(cur, pass)==0){
		system("cls");
		strcpy(updated.uname, username);
		getpass(updated.passwd, "Enter new password: ");
		encrypt(updated.passwd);	
		//copy from user.dat to old.dat
		old=fopen("old.dat", "w");
		ufile=fopen("user.dat", "r");
		while(fread(&tmp, sizeof(user), 1, ufile))
		fwrite(&tmp, sizeof(user), 1, old);
		fclose(old); fclose(ufile);
		//copy back from old.dat to user.dat, replacing the updated user
		ufile=fopen("user.dat", "w");
		old=fopen("old.dat", "r");
		while(fread(&tmp, sizeof(user), 1, old)){
			if(strcmp(tmp.uname, updated.uname)==0)
			fwrite(&updated, sizeof(user), 1, ufile);
			else fwrite(&tmp, sizeof(user), 1, ufile);
		}
		fclose(ufile); fclose(old);
		remove("old.dat");
		system("cls");
		printf("Password changed successfully!");
	}
	else{
		printf("Incorrect password!");
	}
	getch();
}

void adduser(){
	FILE *ufile;
	user newusr;
	char dummyusr[nsize], dummypass[nsize];
	system("cls");
	printf("Enter username for new user: ");
	fflush(stdin);
	gets(newusr.uname);
	if(scanuser(newusr.uname, dummyusr, dummypass)==1){
		system("cls");
		printf("This user already exists!");
		getch();
		return ;
	}
	getpass(newusr.passwd, "Enter password for new user: ");
	encrypt(newusr.passwd);
	ufile=fopen("user.dat", "a");
	fwrite(&newusr, sizeof(user), 1, ufile);
	fclose(ufile);
	system("cls");
	printf("User added successfully!");
	getch();
	return ;
}

void deluser(){
	char uname[nsize];
	FILE *ufile, *old;
	user tmp;
	int found=0;
	system("cls");
	printf("Enter user name to delete: ");
	fflush(stdin);
	gets(uname);
	ufile=fopen("user.dat", "r");
	old=fopen("old.dat", "w");
	while(fread(&tmp, sizeof(user), 1, ufile)){
		if(strcmp(uname, tmp.uname)==0)
		found=1;
		fwrite(&tmp, sizeof(user), 1, old);	
	}
	fclose(old); fclose(ufile);
	if(!found){
		printf("No such user!");
		getch();
		return ;
	}
	ufile=fopen("user.dat", "w");
	old=fopen("old.dat", "r");
	while(fread(&tmp, sizeof(user), 1, old)){
		if(strcmp(tmp.uname, uname)!=0)
		fwrite(&tmp, sizeof(user), 1, ufile);
	}
	fclose(old); fclose(ufile);
	remove("old.dat");
	system("cls");
	printf("User deleted successfully!");
	getch();
	return ;
}

int fdisp(char name[]){				//displays a file
	FILE *fp;
	if(!(fp=fopen(name, "r"))){
		printf("\nfdisplay: File not found!\n");
		return 0;
	}
	while(!feof(fp))
	printf("%c", fgetc(fp));
	fclose(fp);
	return 0;
}

void encrypt(char str[]){			//simple encryption
	int i, len=strlen(str);
	for(i=0; i<len; i++)
		str[i]+=cryptkey;
}

void decrypt(char str[]){
	int i, len=strlen(str);
	for(i=0; i<len; i++)
	str[i]-=cryptkey;
}

int isempty(char file[]){				//checks whether a file is empty
	FILE *fp;
	char ch;
	if(!(fp=fopen(file, "r")))
	fp=fopen(file, "w");			//creates the file if non-existent
	fclose(fp);
	fp=fopen(file, "r");
	if((ch=fgetc(fp))==EOF)
	return 1;						//return 1 if empty, 0 if not
	else return 0;
	fclose(fp);
}

void strlow(char str[]){			//converts string to lowercase
	int i, len=strlen(str);
	for(i=0; i<len; i++)
		str[i]=tolower(str[i]);
}

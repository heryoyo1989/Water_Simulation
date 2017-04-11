# define StrNum 20
# include<string>
class Book
{
  private:
  char name[StrNum];
  char writer[StrNum];
  public:
  Book();
    Book(const char *Name,const char *Writer);
	char *get_name();
	char *get_writer();
	void changeName(const char *newName);
	void changeWriter(const char *newWriter);
	int cmpName(const char *Name);
};

Book::Book()
{
	strcpy(name,"underDefine");
	strcpy(writer,"underDefine");
};
Book::Book(const char *Name,const char *Writer)
{
	strcpy(name,Name);
	strcpy(writer,Writer);
}
char *Book::get_name()
{
	return name;
};
char *Book::get_writer()
{
	return writer;
};
void Book::changeName(const char *newName)
{
	strcpy(name,newName);
};
void Book::changeWriter(const char *newWriter)
{
	strcpy(writer,newWriter);
};
int Book::cmpName(const char *Name)
{
	return strcmp(name,Name);
};





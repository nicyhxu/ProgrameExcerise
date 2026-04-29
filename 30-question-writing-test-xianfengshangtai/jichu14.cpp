#include "stdio.h"
#include <cstring>
#include <iostream>
#include <memory>
using namespace std;

class MyString
{
public:
    MyString() :m_pData(nullptr), m_nLen(0)
    {
        cout << "MyString()" << endl;
    }
    MyString(const char *pStr)   // 允许隐式转换
    {
        cout << "MyString(const char *pStr)" << endl;
        m_nLen = strlen(pStr);
        CopyData(pStr);
    }
    MyString(const MyString& other)
    {
        cout << "MyString(const MyString& other)" << endl;
        if (this != &other)
        {
            m_nLen = other.m_nLen;
            //DeleteData();
            CopyData(other.m_pData);
        }
    }
    //如果赋值操作不返回引用会导致一直构造;
    MyString& operator=(const MyString& other)
    {
        cout << "MyString& operator=(const MyString& other)" << endl;
        if (this != &other)
        {
            m_nLen = other.m_nLen;
            DeleteData();
            CopyData(other.m_pData);
        }
        return *this;
    }

    MyString(MyString&& other)
    {
        cout << "MyString(MyString&& other)" << endl;
        m_nLen = other.m_nLen;
        m_pData = other.m_pData;
        other.m_pData = nullptr;
    }
    //赋值操作符一定要返回
    MyString& operator=(MyString&& other)
    {
        cout << "MyString& operator=(const MyString&& other)" << endl;
        if (this != &other)
        {
            m_nLen = other.m_nLen;
            m_pData = other.m_pData;
            other.m_pData = nullptr;
        }

        return *this;
    }
    MyString operator + (const char *pStr)
    {
        cout << "MyString& operator + (const MyString&& other)" << endl;
        if (1)
        {
            //m_nLen = other.m_nLen;
            //m_pData = other.m_pData;
            //other.m_pData = nullptr;
        }

        return *this;
    }
    ~MyString()
    {
        DeleteData();
    }

private:
    void CopyData(const char *pData)
    {
        if (pData)
        {
            m_pData = new char[m_nLen + 1];
            memcpy(m_pData, pData, m_nLen);
            m_pData[m_nLen] = '\0';
        }
    }

    void DeleteData()
    {
        if (m_pData != nullptr)
        {
            free(m_pData);
            m_pData = nullptr;
        }
    }

private:
    char *m_pData;
    size_t m_nLen;
};
//返回值进行优化，避免了构造函数;
MyString Fun()
{
    //MyString str "hello world";  //隐式转换;
    return MyString("hello world");
}

class A {
public:
    A(int a) : num(a){cout<<"you world 1"<<endl;}
    A(A& a) : num(a.num) {cout<<"you world!"<<endl;}
    //A(const A& a) : num(a.num) {}
    A& operator=(A const & rhs) { return *this; }; // classical implementation
    A& operator=(A&& rhs);
private:
    int num;
};

template <class T, class A1>
std::shared_ptr<T>
factory(const A1& a1)   // one argument version
{
    return std::shared_ptr<T>(new T(a1));
}
// factory3
template <class T, class A1>
std::shared_ptr<T>
factory(A1& a1)
{
    return std::shared_ptr<T>(new T(a1));
}

void f(void fp()) {
cout << is_same<decltype(&fp), void (*)()>();
cout << is_same<decltype(fp), void (*)()>();
}
void pesudo_fp() {}
int  main()
{
	A* q = new A(5);
	int &&a =5;
	std::shared_ptr<A> p = factory<A>(5);
    cout<<"hello world!"<<endl;  
    auto i=10;  
    cout<<i<<endl;  
    MyString str1 = "hello";
    MyString str2(str1);
    //MyString str3;
    MyString  str3 = Fun();
    void (*fp)();
    cout << is_same<decltype(&pesudo_fp), void(*)()>();
	cout << is_same<decltype(fp), void(*)()>();
    return 0;
}
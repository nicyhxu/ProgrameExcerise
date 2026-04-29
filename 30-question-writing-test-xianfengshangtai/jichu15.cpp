#include <iostream>
 
struct X
{
    X() : id(instances++)
    {
        std::cout << "X" << id << ": construct\n";
    }
    
    X(X const& rhs) : id(instances++)
    {
        std::cout << "X" << id << ": <- " << "X" << rhs.id << ": **copy**\n";
        ++copies;
    }
 
    // This particular test doesn't exercise assignment, but for
    // completeness:
    X& operator=(X const& rhs)
    {
        std::cout << "X" << id << ": <- " << "X" << rhs.id << ": assign\n";
    }
    
    ~X() { std::cout << "X" << id << ": destroy\n"; }
 
    unsigned id;
    
    static unsigned copies;
    static unsigned instances;
};
 
unsigned X::copies = 0;
unsigned X::instances = 0;
 
#define CHECK_COPIES( stmt, min, max, comment )                         \
{                                                                       \
    unsigned const old_copies = X::copies;                              \
                                                                        \
    std::cout << "\n" comment "\n" #stmt "\n===========\n";             \
    {                                                                   \
        stmt;                                                           \
    }                                                                   \
    unsigned const n = X::copies - old_copies;                          \
    if (n > max)                                                        \
        std::cout << "*** max is too low or compiler is buggy ***\n";   \
    if (n < min)                                                        \
        std::cout << "*** min is too high or compiler is buggy ***\n";  \
                                                                        \
    std::cout << "-----------\n"                                        \
              << n << "/" << max                                        \
              << " possible copies made\n"                              \
              << max - n << "/" << max - min                            \
              << " possible elisions performed\n\n";                    \
                                                                        \
    if (n > min)                                                        \
        std::cout << "*** " << n - min                                  \
                  << " possible elisions missed! ***\n";                \
}
 
struct trace
{
    trace(char const* name)
        : name(name)
    {
        std::cout << "->: " << name << "\n";
    }
    
    ~trace()
    {
        std::cout << "<-: " << name << "\n";
    }
    
    char const* name;
};
 
void sink(X a)
{
  trace t("sink");
}
 
X nrvo_source()
{
    trace t("nrvo_source");
    X a;
    return a;
}
 
X urvo_source()
{
    trace t("urvo_source");
    return X();
}
 
X identity(X a)
{
    trace t("identity");
    return a;
}
 
X lvalue_;   //返回一个全局变量的引用是可以的,局部变量的引用会报错;
X& lvalue()
{
    return lvalue_;
}
typedef X rvalue;
 
int main()
{
    // Double parens prevent "most vexing parse"
    CHECK_COPIES( X a(( lvalue() )), 1, 1, "Direct initialization from lvalue");
    CHECK_COPIES( X a(( rvalue() )), 0, 1, "Direct initialization from rvalue");
    
    CHECK_COPIES( X a = lvalue(), 1, 1, "Copy initialization from lvalue" );
    CHECK_COPIES( X a = rvalue(), 0, 1, "Copy initialization from rvalue" );
 
    CHECK_COPIES( sink( lvalue() ), 1, 1, "Pass lvalue by value" );
    CHECK_COPIES( sink( rvalue() ), 0, 1, "Pass rvalue by value" );
 
    CHECK_COPIES( nrvo_source(), 0, 1, "Named return value optimization (NRVO)" );
    CHECK_COPIES( urvo_source(), 0, 1, "Unnamed return value optimization (URVO)" );
 
    // Just to prove these things compose properly
    CHECK_COPIES( X a(urvo_source()), 0, 2, "Return value used as ctor arg" );
    
    // Expect to miss one possible elision here
    CHECK_COPIES( identity( rvalue() ), 0, 2, "Return rvalue passed by value" );
}
#include <string>

namespace AA
{
  namespace BB
  {
    //struct D{
	
      struct A{};

       template <class TT,int N> 
      class C
      {
      public :
        //should be found
        C();
        ~C();	
        void a() noexcept ;
        void a(int) throw(A,double);

        //nope, deleted
        C& operator=(const C&) = delete;

        //nope, default
        C(const C&c)=default;

        operator A();
  
        void Z(std::string s="foobar");
  
        //member
        static const int x;

	static int foobarz();

        //nope, has a body
        const double fct1(double chose = 5.){return 5.;}

        //yes, not pure
        virtual const A fct3(const float& truc=5.) const;

        //nope, pure
        virtual const A fct2(const float& truc) const =0;

        //should not be and is not !!
        void foo() const;	

        //template, so no
        template <class T> void bar();
	}; //-- C 
			
    //}; //-- D 
  } //-- BB
} //-- AA 


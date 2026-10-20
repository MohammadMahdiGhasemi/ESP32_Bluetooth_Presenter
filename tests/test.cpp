#include "../ESP32_Presenter/Logic.h"
#include <cassert>
int main(){
 NumberInput n;
 n.digit(2,100);n.digit(5,500);
 assert(n.value==25 && n.count==2);
 assert(!n.ready(1499));assert(n.ready(1500));
 n.clear();assert(!n.ready(9999));
 n.digit(0,0);assert(!n.ready(1000));
 n.clear();n.digit(5,0xffffff00u);assert(n.ready(744));
 n.clear();for(int i=0;i<6;i++)n.digit(9,i);
 assert(n.value==9999 && n.count==4);
}

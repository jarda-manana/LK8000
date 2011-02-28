/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: $
*/

#include "TestContest.h"
#include "Tools.h"
#include <iostream>


int main(int argc, char *argv[])
{
  if(argc < 3) {
    std::cout << "Usage:" << std::endl;
    std::cout << "  TestContest.exe HANDICAP IGC_FILE" << std::endl;
    return EXIT_FAILURE;
  }
  
  try {
    //    unsigned algorithm = CTrace::ALGORITHM_TRIANGLES | CTrace::ALGORITHM_TIME_DELTA;
    unsigned algorithm = CTrace::ALGORITHM_DISTANCE  | CTrace::ALGORITHM_TIME_DELTA;
    //    unsigned algorithm = CTrace::ALGORITHM_DISTANCE | CTrace::ALGORITHM_INHERITED | CTrace::ALGORITHM_TIME_DELTA;
    unsigned size = 250;
    CTestContest test(Convert<unsigned>(argv[1]), argv[2], algorithm, size);
    test.Run();
  }
  catch(const std::exception &ex) {
    std::cerr << "Not handled STD exception: " << ex.what() << "!!!" << std::endl;
    return EXIT_FAILURE;
  }
  catch(...) {
    std::cerr << "Not handled unknown exception!!!" << std::endl;
    return EXIT_FAILURE;
  }
  
  return EXIT_SUCCESS;
}
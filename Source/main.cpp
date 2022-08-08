#include <stdlib.h>
#include <Magic/Magic>
#include "GitLabHook.h"

int main(int argc,char** argv){
    Safe<Magic::Application> application = std::make_shared<Magic::Application>();
    application->initialize();
    return EXIT_SUCCESS;
}

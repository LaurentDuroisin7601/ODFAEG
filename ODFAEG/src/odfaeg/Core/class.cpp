#include "../../../include/odfaeg/Core/class.hpp"
namespace odfaeg {
    namespace core {
        using namespace std;
        Class::Class(std::string name, std::string filePath) : name(name), filePath(filePath) {

        }
        std::vector<std::string> Class::getClasses(std::string path) {
            std::vector<std::string> classes;
            //We get the project directory, and concat it with the folder path.
            std::string appiDir = getCurrentPath()+"\\"+path;
            std::vector<std::string> files;
            //find each header files.
            findFiles(".hpp .h", files, appiDir);
            for (unsigned int i = 0; i < files.size(); i++) {
                //Read header files.
                ifstream ifs(files[i]);
                if (ifs) {
                    std::string line;
                    //Read file's lines.
                    while(getline(ifs, line)) {
                        //check if we find the c++ class keyword and the beginning of a class definition.
                        if (line.find("class") != std::string::npos && line.find("{") != std::string::npos) {
                            //remove spaces at the beginning of the line.
                            while (line.size() > 0 && line.at(0) == ' ') {
                                line.erase(0, 1);
                            }
                            //split the std::string to get the class name.
                            std::vector<std::string> parts = split(line, " ");
                            //remove spaces before the class name.
                            while (parts[1].size() > 0 && parts[1].at(0) == ' ') {
                                parts[1].erase(0, 1);
                            }
                            //add the class name to the vector.
                            classes.push_back(parts[1]);
                        }
                    }
                }
            }
            return classes;
        }
        Class Class::getClass(std::string name, std::string nspc, std::string path) {
            std::string appiDir;
            //If the path is not specified the folder to search c++ classes in is the project's directory.
            if (path == "")
                appiDir = getCurrentPath();
            else
                appiDir = path;
            std::vector<std::string> files;
            //Find headers c++ files in the specified folder's path.
            findFiles(".hpp .h", files, appiDir);
            bool found=false;
            std::string fileContent;
            std::string headerFile;
            std::string namespc="";
            //browse every header files.
            for (unsigned int i = 0; i < files.size() && !found; i++) {
                //Read files.
                ifstream ifs(files[i]);
                if (ifs) {
                    std::string line;
                    fileContent="";
                    //Read lines.
                    while(getline(ifs, line)) {
                        //Ignore c++ comments.
                        if (line.find("/*") != std::string::npos || line.find("/**") != std::string::npos) {
                            while(line.find("*/") == std::string::npos && getline(ifs, line)) {

                            }
                        }
                        //We must also ignore single line comments.
                        if (line.find("//") == std::string::npos && line.find("*/") == std::string::npos) {
                            //Remove template declaration.
                            if (line.find("template ") != std::string::npos) {
                                //check begin and end of template declaration and remove it.
                                int pos = line.find("template ");
                                int pos2 = line.find(">");
                                line.erase(pos, pos2 - pos+1);
                            }
                            //If the template declaration contains other template declarations, we remove them too.
                            if (line.find("<") != std::string::npos) {
                                //check begin and end of template declaration and remove it.
                                int pos = line.find("<");
                                int pos2 = line.find(">");
                                line.erase(pos, pos2 - pos+1);
                            }
                            //split string if the class inherits from base classes.
                            std::vector<std::string> parts = split(line, ":");
                            //if we find the class or struct keywork and the name of the c++ class match, we have found the class.
                            if (parts.size() > 0 && (parts[0].find("class ") != std::string::npos || parts[0].find("struct ") != std::string::npos) && parts[0].find(name) != std::string::npos) {

                                //Remove everything which is after class definition.
                                if (parts[0].find("{") != std::string::npos) {
                                    int pos = parts[0].find("{");
                                    parts[0].erase(pos);
                                }
                                //split the string.
                                std::vector<std::string> parts2 = split(parts[0], " ");
                                //remove space before and after class name.
                                while(parts2[parts2.size()-1].size() > 0 && parts2[parts2.size()-1].at(0) == ' ') {
                                    parts2[parts2.size()-1].erase(0, 1);
                                }
                                while(parts2[parts2.size()-1].size() > 0 && parts2[parts2.size()-1].at(parts2[parts2.size()-1].size()-1) == ' ') {
                                    parts2[parts2.size()-1].erase(parts2[parts2.size()-1].size()-1, 1);
                                }
                                //We have found the class name, we set the header file path and the boolean to stop the loop.
                                if (parts2[parts2.size()-1] == name) {
                                    headerFile = files[i];
                                    found = true;
                                }
                            }
                            //put every lines of the file header file to a string.
                            fileContent += line+"\n";

                        }
                    }
                    ifs.close();

                }
            }
            int j= 0;
            //If we have found the class we check informations about the class.
            if (found) {
                //check each namespaces englobing the class.
                while(fileContent.find("namespace ") != std::string::npos && found) {
                    //Find the namespace pos.
                    unsigned int pos = fileContent.find("namespace ");
                    //Check the namespace name.
                    fileContent = fileContent.substr(pos+9, fileContent.size()-pos-9);

                    while(fileContent.size() > 0 && fileContent.at(0) == ' ') {
                        fileContent.erase(0, 1);
                    }
                    std::vector<std::string> parts = split(fileContent, " ");
                    //We add :: for each sub namespaces found.
                    if (j == 0)
                        namespc += parts[0];
                    else
                        namespc += "::"+parts[0];
                    //we must check if the namespace is declared before the class.
                    pos = fileContent.find_first_of("{");
                    fileContent = fileContent.substr(pos+1, fileContent.size()-pos-1);
                    pos = fileContent.find("namespace ");
                    unsigned int pos2 = fileContent.find(name+" ");

                    found = true;
                    //if there is no more namespace declaration after the class name we can check if the class is in the given namespace.
                    if (pos > pos2) {
                        //if the class is contained in a namespace we must check if a class which the same name is not present in an another namespace.
                        if (nspc != "" && namespc != nspc) {
                            found = false;
                        }
                        //Erase eveything which is before the namespace declaration.
                        fileContent = fileContent.substr(pos2);
                    }
                    j++;
                }
                //We have found the class in the specified namespace, we can get class's informations.
                if (found) {

                    Class cl(name, headerFile);
                    cl.setNamespace(namespc);
                    checkSuperClasses(fileContent, cl);
                    std::string innerClass = "";
                    std::string type = "";
                    int lvl = 0;
                    //At the first recursion the inner class name is empty, the recursion lvl is 0 and the class type is empty.
                    checkInnerClass(innerClass, type, fileContent, lvl,  cl);
                    checkConstructors(fileContent, cl);
                    checkMembersFunctions(fileContent, cl);
                    return cl;
                }
            }
            //Launch an error it the c++ class is not found.
            throw Erreur(70, "Class "+name+"not found in project files!", 3);
        }
        void Class::checkSuperClasses (std::string &fileContent, Class& cl) {


            bool found = false;
            std::string str = fileContent;
            int pos = fileContent.find("{");
            std::string names="";
            //erase everything which is before class definition.
            if (pos != std::string::npos){

                names = fileContent.substr(0, pos);
                fileContent = fileContent.substr(pos+1, fileContent.size() - pos-1);
            }
            if (names != "") {
                //if the base classes are in a nested name, we need to remove remove the :: before splitting with the :.
                std::vector<std::string> parts = split(names, "::");
                std::string str;
                for (unsigned int i = 0; i < parts.size(); i++) {
                    str += parts[i];
                    if (i != parts.size()-1)
                        str+=" ";
                }
                //So we can split to get base classes names specified after the :.
                parts = split(str, ":");
                if (parts.size() > 1) {
                    //split to get all base classes names.
                    parts = split(parts[1], ",");

                    for (unsigned int i = 0; i < parts.size(); i++) {
                        //split to extract nested name spacifiers.
                        std::vector<std::string> parts2 = split(parts[i], " ");
                        if (parts2.size() > 0) {
                            //Remove spaces.
                            while (parts2[parts2.size()-1].size() > 0 && parts2[parts2.size()-1].at(0) == ' ') {
                                parts2[parts2.size()-1] = parts2[parts2.size()-1].erase(0, 1);
                            }
                            while (parts2[parts2.size()-1].size() > 0 && parts2[parts2.size()-1].at(parts2[parts2.size()-1].size()-1) == ' ') {
                                parts2[parts2.size()-1] = parts2[parts2.size()-1].erase(parts2[parts2.size()-1].size()-1, 1);
                            }
                            //Remove template parameters if the base class is template.
                            int pos = parts2[parts2.size()-1].find("<");
                            if (pos != std::string::npos) {
                                parts2[parts2.size()-1].erase(pos);
                            }
                            //we must check if the base class name if different from the current class name.
                            if (cl.getName() != parts2[parts2.size()-1]) {
                                //Get informations about the base class.
                                Class cla = Class::getClass(parts2[parts2.size() - 1]);
                                //add the base class to the vector of base classes.
                                cl.addSuperClass(cla);
                            }
                        }

                    }
                }
            }

        }
        void Class::checkConstructors(std::string& fileContent, Class& cl) {


            //The constructor definition can be at the same file than the constructor declaration, in this case, we need to split the string.
            std::vector<std::string> parts = split(fileContent, "}");
            for (unsigned int i = 0; i < parts.size(); i++) {
                unsigned int nb = 0;
                //Count the number of sub blocks (introduced by if, while, etc...) and remove they definition.
                while(parts[i].find_last_of("{") != std::string::npos) {
                    int pos = parts[i].find_last_of("{");
                    parts[i] = parts[i].erase(pos);
                    nb++;
                }
                //We also need to erase the encapsulation keywords because we don't need it.
                if(parts[i].find("public") != std::string::npos || parts[i].find("private") != std::string::npos || parts[i].find("protected") != std::string::npos) {
                    int pos = parts[i].find(":");
                    parts[i] = parts[i].substr(pos+1);
                    pos = fileContent.find(":");
                    fileContent = fileContent.substr(pos);
                }
                //we also need to split when the constructor definition is not in the header.
                std::vector<std::string> parts2 = split(parts[i], ";");
                for (unsigned int j = 0; j < parts2.size(); j++) {
                    std::string str;
                    //erase constructor name to get constructor arguments.
                    if(parts2[j].find("(") != std::string::npos) {
                        int pos = parts2[j].find("(");
                        str = parts2[j].substr(0, pos+1);
                        //Erase spaces before the \n.
                        while (str.size() > 1 && str.at(str.size()-2) == ' ') {
                            str = str.erase(str.size()-2, 1);

                        }
                    }
                    if (str == "") {
                        str = parts2[j];
                    }
                    //Check if this is an argument list and not a sub block. (of an if by example or something else)
                    if (nb <= 1 && str.find("if(") == std::string::npos && str.find("else") == std::string::npos && str.find("else if(") == std::string::npos && str.find("while(") == std::string::npos && str.find("switch(") == std::string::npos
                        && str.find("for(") == std::string::npos && str.find("do(") == std::string::npos) {
                        //check the position of the constructor argument definition.
                        int index = parts2[j].find("(");
                        if (index != std::string::npos) {
                            //Remove the (.
                            std::string name = parts2[j].substr(0, index);
                            name.erase(0, 1);
                            //Remove spaces and \n at the beginning and at then end.
                            while (name.size() > 0 && name.at(0) == ' ' || name.at(0) == '\n') {
                                name = name.erase(0, 1);
                            }
                            while (name.size() > 0 && name.at(name.size()-1) == ' ' || name.at(name.size()-1) == '\n') {
                                name = name.erase(name.size()-1, 1);
                            }
                            //Check if this is a constructor, in this case the name is the same than the class name.
                            if (name.find(cl.getName()) == 0 && name == cl.getName()) {

                               int pos = parts2[j].find("(");
                               int pos2 = parts2[j].find(")");
                               if (pos != std::string::npos && pos2 != std::string::npos) {
                                   //Get constructor arguments.
                                   std::string types = parts2[j].substr(pos+1, pos2-pos-1);
                                   std::vector<std::string> arguments = split(types, ",");
                                   Constructor constructor(cl.getName());
                                   for (unsigned int i = 0; i < arguments.size(); i++) {
                                        //remove spaces and \n.
                                        while(arguments[i].size() > 0 && arguments[i].at(0) == ' ' || arguments[i].at(0) == '\n') {
                                            arguments[i] = arguments[i].erase(0, 1);
                                        }
                                        //split the arguments.
                                        std::vector<std::string> argTypeName = split(arguments[i], " ");
                                        std::string fullTypeName="";
                                        std::string argName = "";
                                        //we need to check if the argument have a const qualifier.
                                        if (argTypeName[0] == "const") {
                                            fullTypeName = argTypeName[0]+" "+argTypeName[1];
                                            argName = argTypeName[2];
                                        } else {
                                            fullTypeName = argTypeName[0];
                                            argName = argTypeName[1];
                                        }
                                        //Add argument name and argument type.
                                        if (fullTypeName != "") {
                                            constructor.addArgName(argName);
                                            constructor.addArgType(fullTypeName);
                                        }
                                   }
                                   pos = fileContent.find(cl.getName());
                                   //Remove constructor from the string we don't need it anymore we've got every informations needed.
                                   if (pos != std::string::npos) {
                                       //check then end of the
                                       if (fileContent.find("{") != std::string::npos) {
                                           pos2 = fileContent.find("}");
                                       } else {
                                           pos2 = fileContent.find(";");
                                       }
                                       fileContent = fileContent.erase(pos, pos2-pos+1);

                                   }
                                   //add the constructor to the class.
                                   cl.addConstructor(constructor);
                               }
                            }

                        }
                    }
                }
            }
        }
        void Class::checkMembersFunctions(std::string& fileContent, Class& cl) {
            //The function definition can be at the same file than the constructor declaration, in this case, we need to split the string.
            std::vector<std::string> parts = split(fileContent, "}");
            for (unsigned int i = 0; i < parts.size(); i++) {
                unsigned int nb = 0;
                //Count the number of sub blocks (introduced by if, while, etc...) and remove they definition.
                while(parts[i].find_last_of("{") != std::string::npos) {
                    int pos = parts[i].find_last_of("{");
                    parts[i] = parts[i].erase(pos);
                    nb++;
                }
                //we also need to split when the member function definition is not in the header.
                std::vector<std::string> parts2 = split(parts[i], ";");
                for (unsigned int j = 0; j < parts2.size(); j++) {
                    std::string str;
                    if(parts2[j].find("(") != std::string::npos) {
                        int pos = parts2[j].find("(");
                        str = parts2[j].substr(0, pos+1);
                        //Erase spaces before the \n.
                        while (str.size() > 1 && str.at(str.size()-2) == ' ') {
                            str = str.erase(str.size()-2, 1);

                        }
                    }
                    if (str == "") {
                        str = parts2[j];
                    }

                    //Check if this is an argument list and not a sub block. (of an if by example or something else)
                    if (nb <= 1 && str.find("if(") == std::string::npos && str.find("else") == std::string::npos && str.find("else if(") == std::string::npos && str.find("while(") == std::string::npos && str.find("switch(") == std::string::npos
                        && str.find("for(") == std::string::npos && str.find("do(") == std::string::npos) {
                        //check the position of the function member arguments.
                        int index = parts2[j].find("(");
                        if (index != std::string::npos) {
                            //Remove the (.
                            std::string name = parts2[j].substr(0, index);
                            name.erase(0, 1);
                            //remove spaces and \n at the beginning and at the end.
                            while (name.size() > 0 && name.at(0) == ' ' || name.at(0) == '\n') {
                                name = name.erase(0, 1);
                            }
                            while (name.size() > 0 && name.at(name.size()-1) == ' ' || name.at(name.size()-1) == '\n') {
                                name = name.erase(name.size()-1, 1);
                            }
                            //split to get argument list.
                            std::vector<std::string> parts3 = split(name, " ");
                            if (parts3.size() > 1) {

                                MemberFunction mf(parts3[0], parts3[1]);
                                //beginning and end of argument list.
                                int pos = parts2[j].find("(");
                                int pos2 = parts2[j].find(")");
                                if (pos != std::string::npos && pos2 != std::string::npos) {

                                   std::string types = parts2[j].substr(pos+1, pos2-pos-1);

                                   std::vector<std::string> arguments = split(types, ",");
                                   for (unsigned int i = 0; i < arguments.size(); i++) {
                                        while(arguments[i].size() > 0 && arguments[i].at(0) == ' ' || arguments[i].at(0) == '\n') {
                                            arguments[i] = arguments[i].erase(0, 1);
                                        }
                                        std::vector<std::string> argTypeName = split(arguments[i], " ");
                                        std::string fullTypeName="";
                                        //check if there is a const qualifier.
                                        if (argTypeName[0] == "const") {
                                            fullTypeName = argTypeName[0]+" "+argTypeName[1];
                                        } else {
                                            fullTypeName = argTypeName[0];
                                        }
                                        if (fullTypeName != "") {
                                            mf.addArgType(fullTypeName);
                                            mf.addArgName(arguments[arguments.size()-1]);
                                        }
                                   }
                                   pos = fileContent.find(name);
                                   if (pos != std::string::npos) {
                                       //Remove the member function from the string we don't need it anymore after extracting the informations.
                                       if (fileContent.find("{") != std::string::npos) {
                                           pos2 = fileContent.find("}");
                                       } else {
                                           pos2 = fileContent.find(";");
                                       }
                                       fileContent = fileContent.erase(pos, pos2-pos+1);

                                   }
                                   //Add the member function to the class.
                                   cl.addMemberFunction(mf);
                               }
                            }
                        }

                    }
                }
            }
        }
        void Class::checkInnerClass(std::string innerClass, std::string type, std::string& fileContent, int lvl, Class& cl) {
            //Remove template parameters and template template parameters.
            if (fileContent.find("template ") != std::string::npos) {
                int pos = fileContent.find("template ");
                int pos2 = fileContent.find(">");
                fileContent.erase(pos, pos2 - pos+1);
            }
            if (fileContent.find("<") != std::string::npos) {
                int pos = fileContent.find("<");
                int pos2 = fileContent.find(">");
                fileContent.erase(pos, pos2 - pos+1);
            }
            //if we find a subclass we extract informations, care full with the friend class declaration.
            if ((fileContent.find("class ") != std::string::npos || fileContent.find("enum ") != std::string::npos || fileContent.find("struct ") != std::string::npos)
                && fileContent.find("friend") == std::string::npos) {
                std::string type;
                //Check the sublclass type. (enum, calss or struct ?)
                if (fileContent.find("class ") != std::string::npos) {
                    //Remove the class keyword.
                    int pos = fileContent.find("class ");
                    fileContent = fileContent.substr(pos+5, fileContent.size()-pos-5);
                    type = "class";
                } else if (fileContent.find("enum ") != std::string::npos) {
                    //Remove the enum keyword.
                    int pos = fileContent.find("enum ");
                    fileContent = fileContent.substr(pos+4, fileContent.size()-pos-4);
                    type = "enum";
                } else if (fileContent.find("struct ") != std::string::npos) {
                    //Remove the struct keyword.
                    int pos = fileContent.find("struct ");
                    fileContent = fileContent.substr(pos+6, fileContent.size()-pos-6);
                    type = "struct";
                }
                //Remove spaces at the beginning.
                while(fileContent.size() > 0 && fileContent.at(0) == ' ') {
                   fileContent.erase(0, 1);
                }
                //Get the inner class name and check if the inner class have inner classes.
                std::vector<std::string> parts = split(fileContent, " ");
                if (lvl == 0) {
                    checkInnerClass(cl.getName()+"::"+parts[0], type, fileContent, lvl+1, cl);
                } else {
                    checkInnerClass(innerClass + "::"+parts[0], type, fileContent, lvl+1, cl);;
                }
                if (fileContent.find("};") != std::string::npos) {
                    lvl--;
                }
            }
            //If the recursion lvl is > than 0 this is an inner class and we need to add it.
            if (lvl > 0) {
                //Check inner class name in inner class full name.
                std::vector<std::string> parts = split(innerClass, "::");
                //start and end of inner class definition.
                int pos = fileContent.find(parts[parts.size()-1]);
                int pos2 = fileContent.find("};");
                //Add inner class and remove inner class declaration because we don't need it anymore.
                Class innerCl(innerClass, cl.getFilePath());
                fileContent = fileContent.erase(pos, pos2 - pos+2);
                cl.addInnerClass(innerCl);
            }
        }
        void Class::addSuperClass(Class cl) {
            superClasses.push_back(cl);
        }
        void Class::setNamespace(std::string namespc) {
            this->namespc = namespc;
        }
        void Class::addInnerClass(Class innerClass) {
            innerClasses.push_back(innerClass);
        }
        void Class::addConstructor(Constructor c) {
            std::vector<std::string> argsTypes = c.getArgsTypes();
            constructors.push_back(c);
        }
        void Class::addMemberFunction(MemberFunction mf) {
            memberFunctions.push_back(mf);
        }
        std::string Class::getName() {
            return name;
        }
        std::string Class::getFilePath() {
            return filePath;
        }
        std::vector<Constructor> Class::getConstructors() {
            return constructors;
        }
        std::vector<MemberFunction> Class::getMembersFunctions() {
            return memberFunctions;
        }
        std::string Class::getNamespace() {
            return namespc;
        }
        std::vector<Class> Class::getSuperClasses() {
            return superClasses;
        }
    }
}

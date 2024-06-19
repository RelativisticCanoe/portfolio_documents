#include <cmath>
#include <string>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <algorithm>
//#include <windows.h>
#include "celestial_objects.h"

//This was the only thing that was preventing multiple definition errors
const extern std::vector<std::string> celestial_objects::celestial_types_output;
const extern std::vector<std::string> celestial_objects::hubble_types_output;
const extern std::vector<std::string> celestial_objects::stellar_types_output;
const extern std::vector<std::string> celestial_objects::luminosity_class_output;
const extern std::vector<std::string> celestial_objects::parameters_output;

//Storage for keywords for the user interface
enum class commands{Select, Create, Parent, Sort, List, Import, Export, Report, Quit, Help};
const std::vector<std::string> commands_str{"select", "create", "parent", "sort", "list", "import", "export", "report", "quit", "help"};
enum class contexts{Satellite, Catalogue, Object, All, Name, Type, Redshift, Mass, Distance, Magnitude, HubbleClass, StellarClass};
const std::vector<std::string> command_contexts{"satellite", "catalogue", "object", "all", "name", "type", "redshift", "mass", "distance", "magnitude", "hubble_class", "stellar_class"};
const std::vector<char> banned_name_chars{' ', '{', '}', '[', ']'};

//Used to keep track of registered object names, so that none are repeated (so that each name ends up being a unique identifier)
//A lot easier to implement than doing a find over each object

//Holds all imported catalogues of objects
std::vector<celestial_objects::catalogue> catalogues{};

//Used to keep track of which catalogue and object is currently selected
//This allows for run-time selection and manipulation of objects
std::shared_ptr<celestial_objects::catalogue> selected_catalogue_ptr;
std::shared_ptr<celestial_objects::celestial_object> selected_object_ptr;
std::vector<std::shared_ptr<celestial_objects::celestial_object>> selection_ptr;

//Controls the program, should only be "true" when quitting.
bool quit{false};

void user_interface(std::shared_ptr<celestial_objects::catalogue>& selected_catalogue, 
std::shared_ptr<celestial_objects::celestial_object>& selected_object,
std::vector<std::shared_ptr<celestial_objects::celestial_object>>& selection)
{
    /* Run-time interface for users, making use of command words and context words.
    Designed to be more naturalistic than using a commandline interface (hence names are used as unique identifiers) and automatically
    handles the many steps required for a single process such as creating obejcts and parenting
    objects to others.  */
    std::string command_input{""};
    bool valid_command{false};
    int index{0};
    while(!valid_command){
        std::cout << "|";
        if(selected_catalogue.get() != nullptr){
            std::cout << selected_catalogue.get()->get_name();
            if(selected_object.get() != nullptr){
                std::cout << "/" << selected_object.get()->get_name();
            }   
        }

        std::cout << "> ";
        std::cin >> command_input;
        std::vector<std::string>::const_iterator string_location{std::find(commands_str.begin(),
                commands_str.end(), command_input)};
        if(string_location == commands_str.end()){
            std::cout << "Command not recognised. Please reenter your command. " << std::endl;
        } else{
            valid_command = true;
            index = int{string_location - commands_str.begin()};
        }
    }
    commands command{commands(index)};
    std::string context{""};
    std::string name{""};
    switch(command)
    {
        case commands::Select:
        {
            valid_command = false;
            std::cout << "Type 'catalogue' to select a catalogue, 'object' to select an object in the selected catalogue or "
                         << "'selection' to select all objects of a specfic type in the selected catalogue" << std::endl;
            std::cout << "Please enter your selection: ";
            while (!valid_command){
                std::cin >> context;
                std::cout << std::endl;
                if(!(context == "catalogue" || context == "object" || context == "selection")){
                    std::cout << "Invalid input, please enter a valid input: ";
                } else{
                    valid_command = true;
                }
            }

            std::string param_name;
            if(context == "catalogue"){
                std::cout << "Please enter the catalogue name: ";
                std::cin >> param_name;
                std::vector<celestial_objects::catalogue>::iterator catalogue_position{std::find_if(catalogues.begin(),
                catalogues.end(), [&param_name](celestial_objects::catalogue cat){return cat.get_name() == param_name;})};
                if(catalogue_position >= catalogues.end()){
                    std::cout << "Catalogue not found " << std::endl;
                } else{
                    selected_catalogue = std::make_shared<celestial_objects::catalogue>(*catalogue_position);
                    selected_object = std::shared_ptr<celestial_objects::celestial_object>{};
                    selection.clear();
                }
            } else if(context == "object"){
                if(selected_catalogue.get() == nullptr){
                    std::cout << "No catalogue selected. Please select a catalogue. " << std::endl;
                } else{
                    std::cout << "Please enter the name of the object: ";
                    std::cin >> param_name;
                    try{
                        selected_object = selected_catalogue.get()->get_object(param_name);
                    } catch(int e){
                        std::cout << "Object does not exist. Please enter another name. " << std::endl;
                    }
                }
            } else{
                if(selected_catalogue.get() == nullptr){
                    std::cout << "No catalogue selected. Please select a catalogue. " << std::endl;
                } else{
                    bool valid_param{false};
                    while(!valid_param){
                        std::cout << "Please enter the type of object you would like to select: ";
                        std::cin >> param_name;
                        int position{std::find(celestial_objects::celestial_types_output.begin(), celestial_objects::celestial_types_output.end(), param_name) - celestial_objects::celestial_types_output.begin()};
                        if (position >= celestial_objects::celestial_types_output.size()){
                            std::cout << "Invalid type. " << std::endl;
                        } else{
                            valid_param = true;
                            celestial_objects::celestial_types type{celestial_objects::celestial_types(position)};
                            selection = selected_catalogue.get()->subselect_catalogue(type);
                        }

                    }
                }
            }
        }
        break;

        case commands::Create:
        {
            valid_command = false;
            std::cout << "Type 'catalogue' to create a catalogue, or 'object' to create an object in the selected catalogue" << std::endl;
            while (!valid_command){
                std::cin >> context;
                if(!(context == "catalogue" || context == "object")){
                    std::cout << "Invalid input, please enter a valid input: ";
                } else{
                    valid_command = true;
                }
            }

            bool valid_name{false};
            if(context == "catalogue"){
                while(!valid_name){
                    std::cout << "Please enter the name you would like to give the catalogue: ";
                    std::cin >> name;
                    for(int i = 0; i < name.length(); i++){
                        if(name[i] == ':'){
                            name[i] = '_';
                        }
                    }

                    int name_position{std::find_if(catalogues.begin(), catalogues.end(), [&name](celestial_objects::catalogue cat)
                    {return cat.get_name() == name;}) - catalogues.begin()};
                    if(name_position < catalogues.size() && name_position >= 0){
                        std::cout << "Name already taken. Please enter another name. " << std::endl;
                    } else{
                        valid_name = true;
                    }
                }
                celestial_objects::catalogue new_catalogue(name);
                catalogues.push_back(new_catalogue);
            } else{
                if (selected_catalogue.get() == nullptr){
                    std::cout << "No catalogue selected. Please select a catalogue" << std::endl;
                } else{
                    while(!valid_name){
                        std::cout << "Please enter the name you would like to give the object: ";
                        std::cin >> name;
                        for(int i = 0; i < name.length(); i++){
                            if(name[i] == ':' || name[i] == ' '){
                                name[i] = '_';
                            }
                        }

                        std::vector<std::string> obj_names{selected_catalogue.get()->get_obj_names()};

                        int name_position{std::find_if(obj_names.begin(), obj_names.end(), 
                        [&name](std::string current_obj_name){return current_obj_name == name;}) - obj_names.begin()};
                        if(obj_names.size() > 0 && name_position < obj_names.size()){
                            std::cout << "Name already taken. Please enter another name. " << std::endl;
                        } else{
                            valid_name = true;
                        }
                    }

                    bool valid_type{false};
                    while(!valid_type){
                        std::string object_type_str;
                        std::cout << "Please enter the type of the object that will be created: ";
                        std::cin >> object_type_str;
                        int position{std::find(celestial_objects::celestial_types_output.begin(), celestial_objects::celestial_types_output.end(), object_type_str) - celestial_objects::celestial_types_output.begin()};
                        if (position >= celestial_objects::celestial_types_output.size()){
                            std::cout << "No object with that type found" << std::endl;
                        } else if (position == 0){
                            std::cout << "Cannot create an object of the base class. " << std::endl;
                        } else{
                            celestial_objects::catalogue local_catalogue{*selected_catalogue.get()};
                            valid_type = true;
                            celestial_objects::celestial_types object_type{celestial_objects::celestial_types(position)};
                            if(object_type == celestial_objects::celestial_types::Asteroid){
                                celestial_objects::asteroid new_obj(name);
                                local_catalogue.add_object(&new_obj);
                            } else if(object_type == celestial_objects::celestial_types::BlackHole){
                                celestial_objects::black_hole new_obj(name);
                                local_catalogue.add_object(&new_obj);
                            } else if(object_type == celestial_objects::celestial_types::Comet){
                                celestial_objects::comet new_obj(name);
                                local_catalogue.add_object(&new_obj);
                            } else if(object_type == celestial_objects::celestial_types::DwarfPlanet){
                                celestial_objects::dwarf_planet new_obj(name);
                                local_catalogue.add_object(&new_obj);
                            } else if(object_type == celestial_objects::celestial_types::Galaxy){
                                celestial_objects::galaxy new_obj(name);
                                local_catalogue.add_object(&new_obj);
                            } else if(object_type == celestial_objects::celestial_types::GaseousPlanet){
                                celestial_objects::gaseous_planet new_obj(name);
                                local_catalogue.add_object(&new_obj);
                            } else if(object_type == celestial_objects::celestial_types::MainSequenceStar){
                                celestial_objects::main_sequence_star new_obj(name);
                                local_catalogue.add_object(&new_obj);
                            } else if(object_type == celestial_objects::celestial_types::Moon){
                                celestial_objects::moon new_obj(name);
                                local_catalogue.add_object(&new_obj);
                            } else if(object_type == celestial_objects::celestial_types::NeutronStar){
                                celestial_objects::neutron_star new_obj(name);
                                local_catalogue.add_object(&new_obj);
                            } else if(object_type == celestial_objects::celestial_types::Planet){
                                celestial_objects::planet new_obj(name);
                                local_catalogue.add_object(&new_obj);
                            } else if(object_type == celestial_objects::celestial_types::Pulsar){
                                celestial_objects::pulsar new_obj(name);
                                local_catalogue.add_object(&new_obj);
                            } else if(object_type == celestial_objects::celestial_types::RedGiantStar){
                                celestial_objects::red_giant_star new_obj(name);
                                local_catalogue.add_object(&new_obj);
                            } else if(object_type == celestial_objects::celestial_types::Star){
                                celestial_objects::star new_obj(name);
                                local_catalogue.add_object(&new_obj);
                            } else if(object_type == celestial_objects::celestial_types::StellarRemnant){
                                celestial_objects::stellar_remnant new_obj(name);
                                local_catalogue.add_object(&new_obj);
                            } else if(object_type == celestial_objects::celestial_types::Supernova){
                                celestial_objects::supernova new_obj(name);
                                local_catalogue.add_object(&new_obj);
                            } else if(object_type == celestial_objects::celestial_types::TerrestrialPlanet){
                                celestial_objects::terrestrial_planet new_obj(name);
                                local_catalogue.add_object(&new_obj);
                            }
                        std::vector<celestial_objects::catalogue>::iterator catalogue_position{std::find_if(catalogues.begin(),
                        catalogues.end(), [&selected_catalogue](celestial_objects::catalogue cat){return cat.get_name() == selected_catalogue->get_name();})};
                        *catalogue_position = local_catalogue;
                        //local_catalogue.get_object(local_catalogue.get_number() - 1)->get_additional_properties();
                        catalogue_position = std::find_if(catalogues.begin(),
                        catalogues.end(), [&selected_catalogue](celestial_objects::catalogue cat){return cat.get_name() == selected_catalogue->get_name();});
                        selected_catalogue = std::make_shared<celestial_objects::catalogue>(*catalogue_position);
                        //catalogue_position->get_object(catalogue_position->get_number()-1)->get_additional_properties();
                        }
                    }
                }
            }
        }
        break;

        case commands::Parent:
        {
            if(selected_object.get() == nullptr){
                std::cout << "No object selected to parent. " << std::endl;
            }else if(selected_catalogue.get() == nullptr){
                std::cout << "No reference catalogue selected. Please select a catalogue. " << std::endl;
            } else{
                std::cout << "Please enter the name of the object you would like to parent " << selected_object.get()->get_name() << " to: ";
                std::cin >> name;
                int name_position{std::find(selected_catalogue.get()->get_obj_names().begin(), selected_catalogue.get()->get_obj_names().end(), name) - selected_catalogue.get()->get_obj_names().begin()};
                if(name_position < selected_catalogue.get()->get_obj_names().size() && name_position >= 0){
                    std::cout << "Object does not exist. Please enter another name. " << std::endl;
                } else{
                    try{
                        selected_catalogue.get()->get_object(name)->add_member(selected_object);
                    } catch(int e){
                        std::cout << "Object does not exist. Please enter another name. " << std::endl;
                    }
                }
            }
        }
        break;

        case commands::Import:
        {
            celestial_objects::catalogue import_catalogue;
            catalogues.push_back(import_catalogue);
        }
        break;

        case commands::Export:
        {
            selected_catalogue.get()->export_to_file();
        }
        break;

        case commands::Report:
        {
            selected_catalogue.get()->generate_report();
        }
        break;

        case commands::List:
        {
            std::string context{""};
            valid_command = false;
            std::cout << "Enter 'catalogue' to list all catalogues. " << std::endl;
            std::cout << "Enter 'objects' to list all objects in the current catalogue. " << std::endl;
            std::cout << "Enter 'selection' to get all objects in the current selection. " << std::endl;

            while (!valid_command){
                std::cin >> context;
                if(!(context == "catalogue" || context == "objects" || context == "selection")){
                    std::cout << "Invalid input, please enter a valid input: ";
                } else{
                    valid_command = true;
                    if(context == "catalogue"){
                        std::cout << "Catalogues: " << std::endl;
                        for(int i{0}; i < catalogues.size(); i++){
                            std::cout << " - Name: " << catalogues[i].get_name() << ", Number of Objects: " << catalogues[i].get_number() << std::endl;
                        }
                    } else if (context == "objects" && selected_catalogue.get() != nullptr){
                        celestial_objects::celestial_types temp_type{celestial_objects::celestial_types::Unassigned};
                        std::vector<std::shared_ptr<celestial_objects::celestial_object>> temp_vect{selected_catalogue.get()->subselect_catalogue(temp_type)};
                        for(int i{0}; i < temp_vect.size(); i++){
                           std::cout << "- Name: " << temp_vect[i]->get_name() << ", Type: " << celestial_objects::celestial_types_output[int(temp_vect[i]->get_type())] << ", Child Objects: " << temp_vect[i]-> get_member_number() << std::endl;
                        }
                    } else{
                        if(selection.size() > 0){
                            std::cout << "Selection Objects: " << std::endl;
                            for(int i{0}; i < selection.size(); i++){
                                std::cout << "- Name: " << selection[i]->get_name() << ", Type: " << celestial_objects::celestial_types_output[int(selection[i]->get_type())] << ", Child Objects: " << selection[i]-> get_member_number() << std::endl;                                
                            }
                        }
                    }
                    std::cout << std::endl;
                }
            }
        }
        break;

        case commands::Sort:
        {
            if(selected_catalogue.get() != nullptr){
                bool valid_parameter{false};
                std::string param_name;
                while(!valid_parameter){
                    std::cout << "Enter the parameter you would like to sort the catalogue by: " << std::endl;
                    std::cin >> param_name;
                    std::cout << std::endl;
                    int position{std::find(celestial_objects::parameters_output.begin(), celestial_objects::parameters_output.end(), param_name) - celestial_objects::parameters_output.begin()};
                    if(position < 0 || position >= celestial_objects::parameters_output.size()){
                        std::cout << "Invalid parameter" << std::endl;
                    } else{
                        valid_parameter = true;
                        celestial_objects::parameters param{celestial_objects::parameters(position)};
                        selected_catalogue.get()->sort_catalogue(param);
                    }
                }
            }
        }
        break;

        case commands::Help:
        {
            std::cout << "Commands: 'select', 'create', 'parent', 'sort', 'list', 'import', 'export', 'quit' and 'help'." << std::endl;
            std::cout << "Object Types: 'Asteroid', 'BlackHole', 'Comet', 'Galaxy', 'Star', 'MainSequenceStar', 'RedGiantStar', 'StellarRemnant', "
            << "'NeutronStar', 'Pulsar', 'Planet', 'TerrestrialPlanet', 'GaseousPlanet', 'DwarfPlanet', 'Moon'" << std::endl;
            std::cout << "Your selections are presented as '|catalogue_name/object_name>. " << std::endl;
            std::cout << "Enter your commands after the > in the case presented only. " << std::endl;
            std::cout << "When asked to input names, capitalisation and symbols may be used. " << std::endl;
        }
        break;

        case commands::Quit:
        {
            quit = true;
        }
        break;
    }
}

int main()
{
    celestial_objects::catalogue test_catalogue("Test");

    std::cout << "Default Test Objects (in catalogue 'Test'): " << std::endl;

    celestial_objects::galaxy galaxy_test("Test_Galaxy", 0, 0, std::pow(10, 12), 0.001, 0.05, celestial_objects::hubble_types::Sc);
    galaxy_test.get_properties();

    celestial_objects::asteroid aster_test("Test_Asteroid", 0, 0, 1, 0);
    aster_test.get_properties();

    celestial_objects::comet comet_test("Test_Comet", 0, 0, 1, 0.0001);
    comet_test.get_properties();

    celestial_objects::dwarf_planet dplan_test("Test_Dwarf_Planet", 0, 0, 1, 0.001);
    dplan_test.get_properties();

    celestial_objects::moon moon_test("Test_Moon", 0, 0, 1, 1);
    moon_test.get_properties();

    celestial_objects::main_sequence_star msstar_test("Test_Star", 0, 0, 1, 0.0002, celestial_objects::stellar_types::G, 7,
                                                    celestial_objects::luminosity_class::IV, 1, 1);
    msstar_test.get_properties();

    celestial_objects::planet planet_test("Test_Planet", 0, 0, 0.00001, 0.0012);
    planet_test.get_properties();

    celestial_objects::black_hole bh_test("Test_Black_Hole", 0.001, 2000, 3, 0.0012);
    bh_test.get_properties();

    celestial_objects::terrestrial_planet terrplan_test("Test_Terrestrial_Planet", 0, 200, 0.000012, 0.000074);
    terrplan_test.get_properties();

    celestial_objects::gaseous_planet gasplan_test("Test_Gaseous_Planet", 0, 200, 0.000090, 0.0000004);
    gasplan_test.get_properties();

    test_catalogue.add_object(&galaxy_test);
    test_catalogue.add_object(&aster_test);
    test_catalogue.add_object(&comet_test);
    test_catalogue.add_object(&dplan_test);
    test_catalogue.add_object(&moon_test);
    test_catalogue.add_object(&msstar_test);
    test_catalogue.add_object(&planet_test);
    test_catalogue.add_object(&bh_test);
    test_catalogue.add_object(&terrplan_test);
    test_catalogue.add_object(&gasplan_test);

    catalogues.push_back(test_catalogue);

    try{
        test_catalogue.get_object(6)->add_member(test_catalogue.get_object(4), 0.00000012, 4.3, 0.43);
        test_catalogue.get_object(5)->add_member(test_catalogue.get_object(6), 0.00000416, 3.2, 0.12);
    } catch(int e){
        std::cout << "Cannot parent object. " << std::endl;
    }
    std::cout << "James Brady's Astronomical Catalogue Manager" << std::endl;
    std::cout << "--------------------------------------------" << std::endl;
    std::cout << "Commands: 'select', 'create', 'parent', 'sort', 'list', 'import', 'export', 'quit' and 'help'." << std::endl;
    std::cout << "Object Types: 'Asteroid', 'BlackHole', 'Comet', 'Galaxy', 'Star', 'MainSequenceStar', 'RedGiantStar', 'StellarRemnant', "
            << "'NeutronStar', 'Pulsar', 'Planet', 'TerrestrialPlanet', 'GaseousPlanet', 'DwarfPlanet', 'Moon'" << std::endl;
    std::cout << "Your selections are presented as '|catalogue_name/object_name>. " << std::endl;
    std::cout << "Enter your commands after the > in the presented case only. " << std::endl;
    std::cout << "When asked to input names, capitalisation and symbols may be used. " << std::endl;
    std::cout << std::endl;

    while(!quit){
        user_interface(selected_catalogue_ptr, selected_object_ptr, selection_ptr);
    }
    return 0;
}


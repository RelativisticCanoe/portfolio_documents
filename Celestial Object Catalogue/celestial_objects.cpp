/**
 * Definitions for functions included in celestial_objects.h EXCEPT for the get_properties() implementations
 * because they were breaking when I tried to implement them here. I know it goes against convention but
 * I was getting multiple definition errors because of it, and I'd much rather have a functioning project instead
*/

#include "celestial_objects.h"

void celestial_objects::celestial_object::add_member(std::shared_ptr<celestial_object> member_ptr, double orb_distance, double orb_tilt, double orb_eccentricity)
{
    /* Allows a member to be added to the member_objects vector. Inspired by prior experience in using Blender's hierarchy system.
    This implementation is designed for use with the import_from_file() method in catalogue objects, but may also be used directly
    from celestial_object instances as an automatic parameterised constructor for a satellite.
    ADD THE NUMBERS */
    if(member_ptr->parent_object.lock() != nullptr){
        //Children can only be owned singly in satellites and celestial_objects, but shared_ptr is required for the catalogue object
        //This prevents any multiple ownership between non-catalogue objects and allows direct access through children/parents.
        std::cout << "Object is already parented to " << celestial_types_output[int(member_ptr->parent_object.lock().get()->object_type)] << " '" <<
        member_ptr->parent_object.lock()->name <<"'. " << std::endl;
    } else if(member_ptr->parent_object.lock().get() == this){
        //Prevents an object from parenting to itself, this would be a logical paradox (as with self copy assignment)
        std::cout << "Cannot parent an object to itself! " << std::endl;
    } else{
        std::shared_ptr<celestial_object> current_parent{member_ptr->parent_object.lock()};
        while(current_parent.get() != nullptr){
            //Iterates through chain of parents until the top parent is found (it must point to a nullptr parent)
            current_parent = current_parent->parent_object.lock();
        } if(current_parent.get() == member_ptr.get()){
            //Whilst the use of weak_ptr prevents the cyclical error in shared_ptr instances, closed loops are still paradoxical as they are self-ownership
            std::cout << "Cannot parent to object, illegal closed parent/child loop would be created. " << std::endl;
        } else{
            //Adds the satellite to the array of member satellites
            satellite sat(member_ptr, orb_distance, orb_tilt, orb_eccentricity);
            member_objects.push_back(sat);
            member_number++;
        }
    }
}

void celestial_objects::celestial_object::add_member(std::shared_ptr<celestial_object> member_ptr)
{
    if(member_ptr.get()->parent_object.lock() != nullptr){
        std::cout << "Object is already parented to " << celestial_types_output[int(member_ptr.get()->parent_object.lock().get()->get_type())] << " '" <<
        member_ptr.get()->parent_object.lock().get()->get_name() <<"'. " << std::endl;
    } else if(member_ptr.get()->parent_object.lock().get() == this){
        std::cout << "Cannot parent an object to itself! " << std::endl;
    } else{
        std::shared_ptr<celestial_object> current_parent{member_ptr->parent_object.lock()};
        while(current_parent.get() != nullptr){
            current_parent = current_parent->parent_object.lock();
        } if(current_parent.get() == member_ptr.get()){
            std::cout << "Cannot parent to object, illegal closed parent/child loop would be created. " << std::endl;
        } else{
            satellite sat(member_ptr);
            member_objects.push_back(sat);
            member_number++;
        }
    }
}

void celestial_objects::celestial_object::export_to_file(std::fstream& object_dat, std::fstream& relation_dat)
{
    /* Converts the object data and the object's relationships into strings that can be parsed by the catalogue import function.
    These strings are then written to the relevant files. */
    std::stringstream data_string;
    std::stringstream relationship_string;
    data_string << celestial_types_output[int(object_type)] << ":" << name << ":" << redshift << ":" << distance << ":" <<
    mass << ":" << rotational_velocity << '\n';
    object_dat.write(data_string.str().c_str(), data_string.str().size());

    for(int i{0}; i < member_number; i++){
        //Iterates over all children in the member array to produce their relationship data
        //As we know that objects may only have one parent, if all objects are exported, all relationships will be captured
        satellite current_satellite{member_objects[i]};
        relationship_string << name << ":" << current_satellite.get_object()->name << ":" << current_satellite.orbit_distance <<
        ":" << current_satellite.orbit_tilt << ":" << current_satellite.orbit_eccentricity << '\n';
        relation_dat.write(relationship_string.str().c_str(), data_string.str().size());
    }
}

/*
void celestial_objects::celestial_object::remove_member()
{
    //Allows a member to be removed from the member_objects vector at the given position. 
    //Defaults to the last object when no index is given.
    if(member_number > 0){
        member_objects.pop_back();
        member_number--;
    } else{
        std::cout << "There are no satellite members belonging to " << name << std::endl;
        std::cout << "No members removed.";
    }
}
*/

/*
void celestial_objects::celestial_object::remove_member(int& index)
{
    if((member_number > 0) && (index >= 0) && (index < member_number)){
        member_objects.erase(member_objects.begin() + index);
        member_number--;
    } else if(member_number <= 0){
        std::cout << "There are no satellite members belonging to " << name << std::endl;
        std::cout << "No members removed.";
    } else{
        std::cout << "Index " << index << " is out of range of member_objects, size " << member_number << " ." << std::endl;
        std::cout << "Element not removed." << std::endl;
    }
}
*/

celestial_objects::satellite celestial_objects::celestial_object::get_member(int& index)
{
    /* Allows a member to be returned from the member_objects vector at the given position.*/
    if((member_number > 0) && (index >= 0) && (index < member_number)){
        return member_objects[index];
    } else{
        std::cout << "Object index " << index << " out of range for member_objects, size " << member_number << " ," << std::endl;
        std::cout << "Object not returned. " << std::endl;
    }
}

void celestial_objects::celestial_object::get_properties()
{
    /* Will output the properties contained by any celestial object. A function to return derived class-specific
    properties is also included. */

    std::cout << "Name: " << name << std::endl;
    std::cout << "Object Type: " << celestial_types_output[int(object_type)] << std::endl;
    std::cout << "Mass: " << mass << " M_Sun" << std::endl;
    std::cout << "Rotational Velocity: " << rotational_velocity << " rads^-1" << std::endl;
    std::cout << "Distance from Solar System: " << distance << " pc" << std::endl;
    std::cout << "Redshift: " << redshift << std::endl;
    //Returns class-specific properties if present (as in galaxy objects and star object derivatives)
    this->get_additional_properties();
    if(member_objects.size() > 0){
        std::cout << "Children: " << std::endl;
        for(std::vector<celestial_objects::satellite>::iterator i{member_objects.begin()}; i < member_objects.end(); i++){
            std::cout << "- Name: " << i->get_object()->get_name() << ", Type: " << celestial_types_output[int(i->get_object()->get_type())] 
            << ", Number of Children: " << i->get_object()->get_member_number() << std::endl;
            std::cout << "  Orbital Distance: " << i->orbit_distance << " pc, Orbital Tilt: " << i->orbit_tilt << " deg, Orbital Eccentricity: " << i->orbit_eccentricity << std::endl;
        }
    } else{
        std::cout << "No child objects. " << std::endl;
    }
    std::cout << "---------------------------" << std::endl;
}

std::vector<celestial_objects::satellite> celestial_objects::celestial_object::get_all_members()
{
    /* Returns the member_objects vector of a celestial object, which is more convenient than get_member()
    if all or several members are required. */
    return member_objects;
}

std::shared_ptr<celestial_objects::celestial_object> celestial_objects::satellite::get_object()
{
    return satellite_object.lock();
}

void celestial_objects::galaxy::export_to_file(std::fstream& object_dat, std::fstream& relation_dat)
{
    /* Converts the object data and the object's relationships into strings that can be parsed by the catalogue import function.
    These strings are then written to the relevant files. */
    //NEED TO OVERRIDE FOR STARS AND GALAXIES AND THEN I CAN TEST
    std::stringstream data_string;
    std::stringstream relationship_string;
    data_string << celestial_types_output[int(object_type)] << ":" << name << ":" << redshift << ":" << distance << ":" <<
    mass << ":" << rotational_velocity << ":" << stellar_mass_fraction << ":" << hubble_types_output[int(hubble_type)] << '\n';
    object_dat.write(data_string.str().c_str(), data_string.str().size());

    for(int i{0}; i < member_number; i++){
        //Iterates over all children in the member array to produce their relationship data
        //As we know that objects may only have one parent, if all objects are exported, all relationships will be captured
        satellite current_satellite{member_objects[i]};
        relationship_string << name << ":" << current_satellite.get_object()->get_name() << ":" << current_satellite.orbit_distance <<
        ":" << current_satellite.orbit_tilt << ":" << current_satellite.orbit_eccentricity << '\n';
        relation_dat.write(relationship_string.str().c_str(), data_string.str().size());
    }
}

void celestial_objects::galaxy::get_additional_properties()
{
    std::cout << "Hubble Type: " << hubble_types_output[int(hubble_type)] << std::endl;
    std::cout << "Stellar Mass Fraction: " << stellar_mass_fraction << std::endl;
}

void celestial_objects::star::export_to_file(std::fstream& object_dat, std::fstream& relation_dat)
{
    /* Converts the object data and the object's relationships into strings that can be parsed by the catalogue import function.
    These strings are then written to the relevant files. */
    //NEED TO OVERRIDE FOR STARS AND GALAXIES AND THEN I CAN TEST
    std::stringstream data_string;
    std::stringstream relationship_string;
    data_string << celestial_types_output[int(object_type)] << ":" << name << ":" << redshift << ":" << distance << ":" <<
    mass << ":" << rotational_velocity << ":" << stellar_types_output[int(star_type)] << ":" << stellar_digit << ":" << 
    luminosity_class_output[int(luminosity_id)] << ":" << abs_magnitude << ":" << app_magnitude << '\n';
    object_dat.write(data_string.str().c_str(), data_string.str().size());

    for(int i{0}; i < member_number; i++){
        //Iterates over all children in the member array to produce their relationship data
        //As we know that objects may only have one parent, if all objects are exported, all relationships will be captured
        satellite current_satellite{member_objects[i]};
        relationship_string << name << ":" << current_satellite.get_object()->get_name() << ":" << current_satellite.orbit_distance <<
        ":" << current_satellite.orbit_tilt << ":" << current_satellite.orbit_eccentricity << '\n';
        relation_dat.write(relationship_string.str().c_str(), data_string.str().size());
    }
}

void celestial_objects::star::get_additional_properties()
{
    std::cout << "Stellar Classification: " << stellar_types_output[int(star_type)] << stellar_digit 
                << luminosity_class_output[int(luminosity_id)] << std::endl;
    std::cout << "Magnitudes: " << abs_magnitude << " (absolute), " << app_magnitude << " (apparent)" << std::endl;
}

void celestial_objects::catalogue::import_from_file()
{
    //Creates file storage, logical flags and an input
    std::fstream object_data;
    std::fstream relationship_data;
    bool file_read_success{false};
    bool relations_read_success{false};
    std::string file_name{""};
    
    //Loads in the object and relationship files
    //Modified from the basis used in the grades assignent
    while (!file_read_success){
        std::cout << "Enter the filename or path of your .dat file: ";
        std::getline(std::cin, file_name);
        object_data.open(file_name);
        if (!object_data.good()){
            //If the file doesn't exist, it is not loaded and the user is allowed to reenter the path/name
            std::cout << "File or file directory '" << file_name << "' does not exist." << std::endl;
            std::cout << std::endl;
        } else{
            std::cout << "File found successfully!" << std::endl;
            file_read_success = true;
            //Returns 0 if not found => first character used, otherwise gives position of start of file name in path
            int catalogue_name_begin{file_name.find_last_of("/") + 1};
            //-4 accounts for '.dat' extension, hence we are left with the non .dat part of the file name.
            catalogue_name = file_name.substr(catalogue_name_begin, (file_name.length()-4));
            //Modifies the file path to find the relationship data
            int insertion_position{file_name.rfind(".dat")};
            file_name.insert(insertion_position, "_relationships");
            relationship_data.open(file_name);
            if(!relationship_data.good()){
                //Still allows the objects to be loaded in, but still provides a warning if the file is not found
                std::cout << "Object relationship data not found." << std::endl;
                std::cout << "Objects will require manual parenting." << std::endl;
            } else{
                std::cout << "Object relationship data found!" << std::endl;
                relations_read_success = true;
            }
        }
    }

    std::string line;
    //Parser for object data
    bool import_data{true};
    while(std::getline(object_data, line) && import_data){
        if(object_data.eof()){
            import_data = false;
        } else{
            std::string parameter;
            std::vector<std::string> parameter_storage;
            celestial_types object_type{celestial_types::Unassigned};
            std::string object_name;
            double object_redshift;
            double object_distance;
            double object_mass;
            double object_omega;
            std::shared_ptr<celestial_object> object_ptr{nullptr};
            bool read{true};

            while(read){
                //Parameters are delimited by a ':' char within the object file
                //Found parameters are added as strings to a vector of strings
                int break_point{line.find(":")};
                parameter = line.substr(0, break_point);
                parameter_storage.push_back(parameter);
                if(break_point == -1){
                    read = false;
                } else{
                    line.erase(0, break_point+1);
                }
            }

            try{
                //Attemped conversion from parameter strings into useful paremeters
                //If this fails, it is clear that the .dat file loaded in was not configured for this program or there is an error in the file
                //Where possible, data files should be generated by this program via the export_to_file() method and then read in as needed in subsequent sessions.
                std::string object_type_str{parameter_storage[0]};
                int position{std::find(celestial_types_output.begin(), celestial_types_output.end(), object_type_str) - celestial_types_output.begin()};
                object_type = celestial_types(position);

                object_name = parameter_storage[1];
                local_object_names.push_back(object_name);
                object_redshift = std::stod(parameter_storage[2]);
                object_distance = std::stod(parameter_storage[3]);
                object_mass = std::stod(parameter_storage[4]);
                object_omega = std::stod(parameter_storage[5]);

                if(object_type == celestial_types::Galaxy){
                    //Handles a galaxy object due to its unique parameters
                    //There is only one type of galaxy object, so no switch statement is required for construction
                    double object_mass_frac{std::stod(parameter_storage[6])};

                    std::string hubble_type_str{parameter_storage[7]};
                    position = int(std::find(hubble_types_output.begin(), hubble_types_output.end(), hubble_type_str) - hubble_types_output.begin());
                    hubble_types object_htype{hubble_types(position)};

                    galaxy object(object_name, object_redshift, object_distance, object_mass, object_omega, object_mass_frac, object_htype);
                    object_ptr = std::make_shared<galaxy>(object);

                } else if(object_type == celestial_types::Star || object_type == celestial_types::MainSequenceStar || object_type == celestial_types::RedGiantStar 
                || object_type == celestial_types::StellarRemnant || object_type == celestial_types::NeutronStar ||  object_type == celestial_types::Pulsar){
                    //Handles stellar objects and parses their extra paremeters
                    //The default case does not throw an error, as we know from the else if above that the type must be within those checked by the else if() above.
                    std::string stel_type_str{parameter_storage[6]};
                    position = int(std::find(stellar_types_output.begin(), stellar_types_output.end(), stel_type_str) - stellar_types_output.begin());
                    stellar_types stel_type{stellar_types(position)};

                    int stel_digit{std::stoi(parameter_storage[7])};

                    std::string lum_no_str{(parameter_storage[8])};
                    position = int(std::find(luminosity_class_output.begin(), luminosity_class_output.end(), lum_no_str) - luminosity_class_output.begin());
                    luminosity_class lum_no{luminosity_class(position)};

                    double absolute_lum{std::stod(parameter_storage[9])};
                    double apparent_lum(std::stod(parameter_storage[10]));

                    //Whilst extensive, this switch handles the construction of individual objects via identification of object type
                    switch(object_type)
                    {
                        //Handles all stellar objects as they have more parameters due to being luminous
                        case celestial_types::RedGiantStar:
                        {
                            red_giant_star object(object_name, object_redshift, object_distance, object_mass, object_omega, stel_type,
                            stel_digit, lum_no, absolute_lum, apparent_lum);
                            object_ptr = std::make_shared<red_giant_star>(object);
                        }
                        break;

                        case celestial_types::MainSequenceStar:
                        {
                            main_sequence_star object(object_name, object_redshift, object_distance, object_mass, object_omega, stel_type,
                            stel_digit, lum_no, absolute_lum, apparent_lum);
                            object_ptr = std::make_shared<main_sequence_star>(object);
                        }
                        break;

                        case celestial_types::NeutronStar:
                        {
                            neutron_star object(object_name, object_redshift, object_distance, object_mass, object_omega, stel_type,
                            stel_digit, lum_no, absolute_lum, apparent_lum);
                            object_ptr = std::make_shared<neutron_star>(object);
                        }
                        break;

                        case celestial_types::Pulsar:
                        {
                            pulsar object(object_name, object_redshift, object_distance, object_mass, object_omega, stel_type,
                            stel_digit, lum_no, absolute_lum, apparent_lum);
                            object_ptr = std::make_shared<pulsar>(object);
                        }
                        break;

                        case celestial_types::StellarRemnant:
                        {
                            stellar_remnant object(object_name, object_redshift, object_distance, object_mass, object_omega, stel_type,
                            stel_digit, lum_no, absolute_lum, apparent_lum);
                            object_ptr = std::make_shared<stellar_remnant>(object);                                    
                        }
                        break;

                        default:
                        {
                            star object(object_name, object_redshift, object_distance, object_mass, object_omega, stel_type,
                            stel_digit, lum_no, absolute_lum, apparent_lum);
                            object_ptr = std::make_shared<star>(object);                                    
                        }
                        break;                                
                    }
                } else{
                    //Handles all other derived classes not derived from the star class
                    switch (object_type)
                    {
                        case celestial_types::Asteroid:
                        {
                            asteroid object(object_name, object_redshift, object_distance, object_mass, object_omega);
                            object_ptr = std::make_shared<asteroid>(object);
                        }
                        break;
                
                        case celestial_types::BlackHole:
                        {
                            black_hole object(object_name, object_redshift, object_distance, object_mass, object_omega);
                            object_ptr = std::make_shared<black_hole>(object);
                        }
                        break;
                
                        case celestial_types::Comet:
                        {
                            comet object(object_name, object_redshift, object_distance, object_mass, object_omega);
                            object_ptr = std::make_shared<comet>(object);
                        }
                        break;
                
                        case celestial_types::DwarfPlanet:
                        {
                            dwarf_planet object(object_name, object_redshift, object_distance, object_mass, object_omega);
                            object_ptr = std::make_shared<dwarf_planet>(object);
                        }
                        break;
                
                        case celestial_types::GaseousPlanet:
                        {
                            gaseous_planet object(object_name, object_redshift, object_distance, object_mass, object_omega);
                            object_ptr = std::make_shared<gaseous_planet>(object);
                        }
                        break;
                
                        case celestial_types::Moon:
                        {
                            moon object(object_name, object_redshift, object_distance, object_mass, object_omega);
                            object_ptr = std::make_shared<moon>(object);
                        }
                        break;
                
                        case celestial_types::Planet:
                        {
                            planet object(object_name, object_redshift, object_distance, object_mass, object_omega);
                            object_ptr = std::make_shared<planet>(object);
                        }
                        break;
                
                        case celestial_types::TerrestrialPlanet:
                        {
                            terrestrial_planet object(object_name, object_redshift, object_distance, object_mass, object_omega);
                            object_ptr = std::make_shared<terrestrial_planet>(object);
                        }
                        break;
                
                        default:
                        {
                            //If all else fails and an object cannot be created, an error will be thrown by default.
                            throw int{-1};
                        }
                        break;
                    }
                }

                catalogue_objects.push_back(std::shared_ptr<celestial_object>{object_ptr});
                object_amount++;     
            } catch(std::bad_alloc){
                //Whilst unlikely on modern hardware, this will catch any cases where there is not enough memory left in RAM to assign an object.
                std::cout << "Not enough memory available to allocate to object." << std::endl;
            } catch(std::invalid_argument const& exception){
                //This should handle bad assignments due to incorrect data
                //Theoretically, if this isn't thrown the constructor for each class SHOULD work.
                std::cout << "ERROR: " << exception.what() << std::endl;
            } catch(int i){
                std::cout << "Unable to create object of unknown type  " << parameter_storage[0] << " ." << std::endl;
            }
        }
    }

    //As above, but parses data for relationships from its corresponding file to construct the parent/child hierarchy.
    //This will not run if the relationships data file is not found.
    if(relationship_data.good()){
        bool import_relationships{true};
        while(std::getline(relationship_data, line) && import_relationships){
            if(relationship_data.eof()){
                import_relationships = false;
            } else{
                std::string parameter;
                std::vector<std::string> parameter_storage;
                celestial_types object_type{celestial_types::Unassigned};
                std::string parent_name;
                std::string child_name;
                double orbital_distance;
                double orbital_tilt;
                double orbital_eccentricity;
                std::shared_ptr<celestial_object> parent_ptr{nullptr};
                std::shared_ptr<celestial_object> child_ptr{nullptr};

                //HERE'S THE GDAMN ISSUE
                bool read{true};
                while(read){
                    int break_point{line.find(":")};
                    parameter = line.substr(0, break_point);
                    parameter_storage.push_back(parameter);
                    if(break_point == -1){
                        read = false;
                    } else{
                        line.erase(0, break_point+1);
                    }
                }

                parent_name = parameter_storage[0];
                child_name = parameter_storage[1];
                orbital_distance = std::stod(parameter_storage[2]);
                orbital_tilt = std::stod(parameter_storage[3]);
                orbital_eccentricity = std::stod(parameter_storage[4]);

                //Object names are used as their unique identifiers, hence the current_object_names vector in the main .cpp file
                //If an object with a certain name cannot be found in the catalogue, it does not exist and hence cannot be made a parent/child
                std::vector<std::shared_ptr<celestial_object>>::iterator parent_position{std::find_if(catalogue_objects.begin(),
                catalogue_objects.end(), [&parent_name](const std::shared_ptr<celestial_object> ptr){return ptr.get()->get_name() == parent_name;})};
                std::vector<std::shared_ptr<celestial_object>>::iterator child_position{std::find_if(catalogue_objects.begin(),
                catalogue_objects.end(), [&child_name](const std::shared_ptr<celestial_object> ptr){return ptr.get()->get_name() == child_name;})};
                if(parent_position == catalogue_objects.end()){
                    std::cout << "Cannot find parent object '" << parent_name << "'." << std::endl;
                } else if(child_position == catalogue_objects.end()){
                    std::cout << "Cannot find child object '" << child_name << "'." << std::endl;
                } else{
                    //Creates a satellite object within the parent object's member_objects array, pointing to the child object, given that they both exist
                    parent_ptr = *parent_position;
                    child_ptr = *child_position;
                    parent_ptr->add_member(child_ptr, orbital_distance, orbital_tilt, orbital_eccentricity);
                }
            }
        }
    }
    //Makes sure that the files are closed and hence memory is released back to the system
    object_data.close();
    relationship_data.close();
}

void celestial_objects::catalogue::export_to_file()
{
    /* Allows a catalogue to be exported to a file. FAR simpler (and hence shorter) than trying to import a catalogue
    as everything is cast to strings and saved to lines regardless of object. */
    //Create file "<catalogue_name>.txt" using fstream
    //Create file "<catalogue_name>_parents.txt" using fstream
    //Need to pass fstream to export methods
    std::fstream object_export;
    std::fstream relationship_export;
    if(!std::filesystem::exists(catalogue_name + ".dat")){
        //Creates the data files if non-existent
        std::cout << "File '" << catalogue_name << ".dat' does not exist. " << std::endl;
        std::cout << "Creating file in local directory... " << std::endl;
        object_export.open(catalogue_name + ".dat", std::ios::out | std::ios::trunc);
        std::cout << "File created! " << std::endl;
        if(!std::filesystem::exists(catalogue_name + "_relationships.dat")){
            std::cout << "Creating relationship data file in local directory... " << std::endl;
            relationship_export.open(catalogue_name + "_relationships.dat", std::ios::out | std::ios::trunc);
            std::cout << "Relationship data file created!" << std::endl;
        }
    } else{
        //Just warns against files with pre-existing data and gives the option to prevent
        std::cout << "WARNING: File '" << catalogue_name << ".dat' alrady exists in the local directory and contains data." << std::endl;
        std::cout << "Would you like to overwrite '" << catalogue_name << ".dat'? [Y/N]   ";
        char input;
        bool valid_input{false};
        while(!valid_input){
            std::cin >> input;
            std::cout << std::endl;
            if(!(std::tolower(input) == 'y' || std::tolower(input) == 'n') || std::cin.fail()){
                std::cout << "Invalid input. Please input either 'Y' or 'N'. [Y/N]   ";
                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            } else{
                valid_input = true;
            }
        } if(std::tolower(input) == 'n'){
            //Timestamps help fix the issue of creating a unique file name when exporting if an export is desired whilst not overwriting the base files.
            //object_export.close();
            //relationship_export.close();
            std::time_t current{std::time(0)};
            std::tm now{current};
            std::stringstream timestamp;
            timestamp << now.tm_yday << now.tm_mon << now.tm_year << "_" << now.tm_hour << now.tm_min << now.tm_sec;
            object_export.open(catalogue_name + timestamp.str() +".dat", std::ios::out | std::ios::trunc);
            std::cout << "Timestamped data file created!" << std::endl;
            relationship_export.open(catalogue_name + timestamp.str() +"_relationships.dat", std::ios::out | std::ios::trunc);
            std::cout << "Timestamped relationships data file created!" << std::endl;
        } else{
            //Reopens files in write-only, truncate mode if overwriting
            //object_export.close();
            object_export.open(catalogue_name + ".dat", std::ios::out | std::ios::trunc);
            //relationship_export.close();
            relationship_export.open(catalogue_name + "_relationships.dat", std::ios::out | std::ios::trunc);
        }
    }

    //Goes through all objects conatined in a catalogue and calls their export functions to write their data to the open files
    for(catalogue_position = catalogue_objects.begin(); catalogue_position < catalogue_objects.end(); catalogue_position++){
        std::shared_ptr<celestial_object> current_object_ptr{*catalogue_position};
        current_object_ptr->export_to_file(object_export, relationship_export);
    }
    
    //Closes the files
    object_export.close();
    relationship_export.close();
}

//CHANGE TO SHARED POINTER 
void celestial_objects::catalogue::add_object(celestial_object* object)
{
    std::shared_ptr<celestial_object> object_ptr{object};
    catalogue_objects.push_back(object_ptr);
    local_object_names.push_back(object_ptr.get()->get_name());
    object_amount++;
}

void celestial_objects::catalogue::sort_catalogue(parameters& parameter)
{
    try 
    {
        switch (parameter)
        {
            case celestial_objects::parameters::Name:
            {
                std::sort(catalogue_objects.begin(), catalogue_objects.end(), 
                [&](std::shared_ptr<celestial_object> const i, 
                std::shared_ptr<celestial_object> const j)
                {return celestial_objects::name_sort(i->get_name(), j->get_name());});
            }
            break;

            case celestial_objects::parameters::Distance:
            {
                std::sort(catalogue_objects.begin(), catalogue_objects.end(), 
                [&](std::shared_ptr<celestial_object> const i, 
                std::shared_ptr<celestial_object> const j)
                {return i->distance < j->distance;});
            }
            break;

            case celestial_objects::parameters::Mass:
            {
                std::sort(catalogue_objects.begin(), catalogue_objects.end(), 
                [&](std::shared_ptr<celestial_object> const i, 
                std::shared_ptr<celestial_object> const j)
                {return i->mass < j->mass;});
            }
            break;

            case celestial_objects::parameters::Redshift:
            {
                std::sort(catalogue_objects.begin(), catalogue_objects.end(), 
                [&](std::shared_ptr<celestial_object> const i, 
                std::shared_ptr<celestial_object> const j)
                {return i->redshift < j->redshift;});
            }
            break;

            case celestial_objects::parameters::RotationalVelocity:
            {
                std::sort(catalogue_objects.begin(), catalogue_objects.end(), 
                [&](std::shared_ptr<celestial_object> const i, 
                std::shared_ptr<celestial_object> const j)
                {return i->rotational_velocity < j->rotational_velocity;});
            }
            break;

            case celestial_objects::parameters::MemberNumber:
            {
                std::sort(catalogue_objects.begin(), catalogue_objects.end(), 
                [&](std::shared_ptr<celestial_object> const i, 
                std::shared_ptr<celestial_object> const j)
                {return i->member_number < j->member_number;});
            }
            break;

            default:
            {
                throw int{-1};
            }
            break;
        }

        //Sorts the names afterwards
        for(std::vector<std::shared_ptr<celestial_object>>::iterator i{catalogue_objects.begin()}; i < catalogue_objects.end(); i++){
            local_object_names[int(i - catalogue_objects.begin())] = i->get()->get_name();
        }
    } catch(int e){
        std::cout << "Cannot sort a full catalogue by special parameter. ";
    }
}

std::shared_ptr<celestial_objects::celestial_object> celestial_objects::catalogue::get_object(std::string name)
{
    std::vector<std::string>::iterator object_position{std::find_if(local_object_names.begin(),
    local_object_names.end(), [&name](const std::string str){return str == name;})};
    if(object_position >= local_object_names.end()){
        std::cout << "Object not found, please enter another name. ";
        throw(-1);
    } else{
        return catalogue_objects[int(object_position - local_object_names.begin())];
    }
}

std::shared_ptr<celestial_objects::celestial_object> celestial_objects::catalogue::get_object(int index)
{
    if(index < 0 || index >= catalogue_objects.size()){
        std::cout << "Index out of range. " << std::endl;
        throw(-1);
    } else{
        return catalogue_objects[index];
    }
}

std::vector<std::shared_ptr<celestial_objects::celestial_object>> celestial_objects::catalogue::subselect_catalogue(celestial_objects::celestial_types& type)
{
    std::vector<std::shared_ptr<celestial_objects::celestial_object>> subselection;
    switch(type)
    {
        //This switch statement accounts for the derived classes, so that derived objects are captured when their base classes are requested
        case celestial_objects::celestial_types::Star:
        {
            for(catalogue_position = catalogue_objects.begin(); catalogue_position < catalogue_objects.end(); catalogue_position++){
                celestial_objects::celestial_types o_type{catalogue_position->get()->object_type};
                if(o_type == celestial_objects::celestial_types::Star || o_type == celestial_objects::celestial_types::MainSequenceStar || 
                o_type == celestial_objects::celestial_types::RedGiantStar || o_type == celestial_objects::celestial_types::StellarRemnant || 
                o_type == celestial_objects::celestial_types::NeutronStar || o_type == celestial_objects::celestial_types::Pulsar){
                    subselection.push_back(*catalogue_position);
                }
            }
        }
        break;

        case celestial_objects::celestial_types::StellarRemnant:
        {
            for(catalogue_position = catalogue_objects.begin(); catalogue_position < catalogue_objects.end(); catalogue_position++){
                celestial_objects::celestial_types o_type{catalogue_position->get()->object_type};
                if(o_type == celestial_objects::celestial_types::StellarRemnant || o_type == celestial_objects::celestial_types::NeutronStar || 
                o_type == celestial_objects::celestial_types::Pulsar){
                    subselection.push_back(*catalogue_position);
                }
            }
        }
        break;

        case celestial_objects::celestial_types::NeutronStar:
        {
            for(catalogue_position = catalogue_objects.begin(); catalogue_position < catalogue_objects.end(); catalogue_position++){
                celestial_objects::celestial_types o_type{catalogue_position->get()->object_type};
                if(o_type == celestial_objects::celestial_types::NeutronStar || o_type == celestial_objects::celestial_types::Pulsar){
                    subselection.push_back(*catalogue_position);
                }
            }
        }
        break;

        case celestial_objects::celestial_types::Planet:
        {
            for(catalogue_position = catalogue_objects.begin(); catalogue_position < catalogue_objects.end(); catalogue_position++){
                celestial_objects::celestial_types o_type{catalogue_position->get()->object_type};
                if(o_type == celestial_objects::celestial_types::Planet || o_type == celestial_objects::celestial_types::TerrestrialPlanet || 
                o_type == celestial_objects::celestial_types::GaseousPlanet || o_type == celestial_objects::celestial_types::DwarfPlanet){
                    subselection.push_back(*catalogue_position);
                }
            }
        }
        break;

        case celestial_objects::celestial_types::Unassigned:
        {
            subselection = catalogue_objects;
        }
        break;

        //This accouunts for any class without derivatives
        default:
        {
            for(catalogue_position = catalogue_objects.begin(); catalogue_position < catalogue_objects.end(); catalogue_position++){
                if(catalogue_position->get()->object_type == type){
                    subselection.push_back(*catalogue_position);
                }
            }
        }
        break;
    }
    
    return subselection;
}

void celestial_objects::catalogue::generate_report()
{
    //Get number of objects
    //Average distance and redshift
    //Get number of each type
    //
    std::cout << "Catalogue: " << catalogue_name << std::endl;
    std::cout << "Total number of objects: " << object_amount << std::endl;
    std::cout << "Object information: " << std::endl;
    std::cout << "----------------------------" << std::endl;
    for(std::vector<std::shared_ptr<celestial_objects::celestial_object>>::iterator i{catalogue_objects.begin()}; i < catalogue_objects.end(); i++ ){
        i->get()->get_properties();
        std::cout << std::endl;
    }
}

std::string celestial_objects::catalogue::get_name()
{
    return catalogue_name;
}

bool celestial_objects::name_sort(std::string name_a, std::string name_b)
{
    for(int i{0}; i < name_a.size(); i++){
        name_a[i] = std::tolower(name_a[i]);
    } for(int i{0}; i < name_b.size(); i++){
        name_b[i] = std::tolower(name_b[i]);
    }
    return name_a < name_b;
}

bool celestial_objects::numerical_sort(int& a, int& b)
{
    return a < b;
}

bool celestial_objects::numerical_sort(double& a, double& b)
{
    return a < b;
}

bool celestial_objects::hubble_sort(celestial_objects::hubble_types& hubtype_a, celestial_objects::hubble_types& hubtype_b)
{
    int a{int(hubtype_a)};
    int b{int(hubtype_b)};
    return a < b;
}

bool celestial_objects::stellar_sort(celestial_objects::stellar_types& steltype_a, celestial_objects::stellar_types& steltype_b)
{
    int a{int(steltype_a)};
    int b{int(steltype_b)};
    return a < b;
}
/**
 * Header file for all of the objects I have defined for the celestial object catalogue.
 * These objects are designed to be used with as few restrictions as possible and with
 * as few attributes as possible.
 * 
 * Need to change pointers between parents and children to weak_ptrs to prevent circular references and hence
 * prevent memory leaks. We only want a shared_ptr so we can access the objects from the catalogue BUT we want
 * that to be the only counted reference => use weak_ptrs within non-catalogue objects to allow this to always
 * be the case. Will want to skip over the satellite objects, as they contain the positional info, which is useful
 * going down, not so useful going up.
 * 
 * Parenting requires a few checks:
 * 1) the object is not already parented => weak_ptr to parent
 * 2) the object is not attempting to parent itself (logical paradox!) => just like copy constructor
 * 3) the object is not parenting itself to make a chain => whilst weak_ptr gets rid of the obvious issue, it's still inconvenient
 *    => go all the way up the chain to the top parent and check that it's not the same as the object that will be parented
 *       we know that it can't be already parented, so it won't appear in any children.
 * 4) that the parent chain is logical => galaxies => stellar objects => planets => moons
 *                                                                    => non-planets
 *                                                 => other stars OF LOWER MASS
 *    this should help prevent the issues in 3) but the check should still remain regardless     
*/

#ifndef CELESTIALOBJECTS_H
#define CELESTIALOBJECTS_H

#include <vector>
#include <string>
#include <iostream>
#include <memory>
#include <fstream>
#include <filesystem>
#include <algorithm>
#include <ctime>

namespace celestial_objects
{   
    /* Contains all of the methods, variables and classes relevant to the celestial objects, to avoid any confusion with another
    namespace such as std, as outlined in the module. */

    //Might want to define a celestial coord class
    //May also want to define a celestial distance class, to preserve accuracy (long doubles may be useful too).
        //AU, pc, kpc and Mpc would be very useful.
    //May also want to define a way to solve orbits via gravitational attraction as requested?
    //May also want to create viewable diagrams.
    //If I have time, maybe add a UI for the program too?

    //Enum classes for setting types within a defined range and their corresponding vectors of string outputs.
    //Expanded from implementation in the the previous galaxy assignment.
    //Enum class for derivative objects of celestial_object

    enum class celestial_types{Unassigned, Galaxy, Star, MainSequenceStar, RedGiantStar,
    Planet, TerrestrialPlanet, GaseousPlanet, DwarfPlanet, Moon, Comet, Asteroid, Satellite, StellarRemnant, Supernova, NeutronStar, Pulsar, BlackHole};
    const std::vector<std::string> celestial_types_output{"Unassigned", "Galaxy", "Star", "MainSequenceStar", "RedGiantStar", 
                                                    "Planet", "TerrestrialPlanet", "GaseousPlanet", "Dwarf Planet", "Moon", "Comet", "Asteroid", "Satellite",
                                                    "StellarRemnant", "Supernova", "NeutronStar", "Pulsar", "BlackHole"};

    //Enum class for hubble types of galaxies
    //This was taken from the previous galaxies assignment, if it ain't broke, don't fix it.
    enum class hubble_types{Unassigned, E0, E1, E2, E3, E4, E5, E6, E7, S0, Sa, Sb, Sc, SBa, SBb, SBc, Irr};
    const std::vector<std::string> hubble_types_output{"Unassigned", "E0", "E1", "E2", "E3", "E4", "E5", "E6", "E7", "S0", "Sa", "Sb", "Sc",
                                      "SBa", "SBb", "SBc", "Irr"};

    //Enum class for stellar types 
    enum class stellar_types{Unassigned, O, B, A, F, G, K, M};
    const std::vector<std::string> stellar_types_output{"Unassigned", "O", "B", "A", "F", "G", "K", "M"};

    //Enum class for luminosity class of stellar objects
    enum class luminosity_class{Unassigned, Zero, IaPlus, Ia, Iab, Ib, II, III, IV, V, VI, VII};
    const std::vector<std::string> luminosity_class_output{"Unassigned", "0", "Ia+", "Iab", "Ib", "II", "III", "IV",
                                                     "V", "VI", "VII"};

    enum class parameters{Name, CelestialType, HubbleType, StellarType, Redshift, Distance, Mass, RotationalVelocity, MemberNumber};
    const std::vector<std::string> parameters_output{"Name", "CelestialType", "HubbleType", "StellarType", "Redshift", "Mass", "RotationalVelocity", "MemberNumber"};
    class celestial_object;
    class satellite;
    class catalogue;

    class celestial_object
    {
        /* Acts as an abstract class for all celestial objects e.g. stars, galaxies, etc. The key data is included in
        this class, being: name, object type, redshift, distance, mass, rotational velocity and a vector orbiting objects, as these can apply to
        almost any celestial object. In the case that a celestial object doesn't have any orbiting objects, the vector remains empty
        and hence adds minimal overhead. Also includes the celestial coordinates of an object, so that they can be located. */
        protected:
            celestial_types object_type{celestial_types::Unassigned};
            std::string name{"Unassigned"};
            double redshift{0};
            double distance{0};
            double mass{0};
            double rotational_velocity{0};
            //double right_ascension{0};
            //double declination{};
            //Vector for orbiting/bound objects
            std::weak_ptr<celestial_object> parent_object;
            std::vector<celestial_objects::satellite> member_objects;
            int member_number{0};

        public:
            friend class catalogue;
            celestial_object() = default;

            //This is not the default unparameterised constructor, but because names are used as unique identifiers this is used instead
            //Hence, names can be checked outside of the constructor and the rest of the input parameters can be handled here
            celestial_object(std::string name_input)
            {
                std::swap(this->name, name_input);
                std::cout << "Enter the redshift of the object (between -1 and 14): ";
                std::cin >> redshift;
                std::cout<< std::endl;
                while(std::cin.fail() || (redshift < -1 || redshift > 10)){
                    std::cin.clear();
                    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                    std::cout << "Please enter a valid redshift value: ";
                    std::cin >> redshift;
                    std::cout<< std::endl;
                }
                
                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                std::cout << "Enter the distance to the object (in pc, up to 10 Gpc): ";
                std::cin >> distance;
                std::cout<< std::endl;
                while(std::cin.fail() || (distance < 0 || distance > 10000000000)){
                    std::cin.clear();
                    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                    std::cout << "Please enter a valid distance value: ";
                    std::cin >> distance;
                    std::cout<< std::endl;
                }

                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                std::cout << "Enter the object's mass (in solar masses): ";
                std::cin >> mass;
                std::cout<< std::endl;
                while(std::cin.fail() || (mass < 0 || mass > 1000000000000000000)){
                    std::cin.clear();
                    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                    std::cout << "Please enter a valid mass value: ";
                    std::cin >> mass;
                    std::cout<< std::endl;
                }
                
                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                std::cout << "Please input the object's rotational velocity (up to 10000 rads^-1): ";
                std::cin >> rotational_velocity;
                std::cout<< std::endl;
                while(std::cin.fail() || (rotational_velocity < 0 || rotational_velocity > 10000)){
                    std::cin.clear();
                    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                    std::cout << "Please type a valid mass fraction: ";
                    std::cin >> rotational_velocity;
                    std::cout<< std::endl;
                }
                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            }

            //Most constructors are defined here as many of the classes share similar parameters, hence constructing from base class reference is very useful 
            celestial_object(std::string name_input, double z, double dist, double m, double omega)
            {
                name = name_input;
                redshift = z;
                distance = dist;
                mass = m;
                rotational_velocity = omega;
            }

            //Copy constructor and assignment operator
            celestial_object(const celestial_object& object)
            {
                this->object_type = object.object_type;
                this->name = object.name;
                this->redshift = object.redshift;
                this->distance = object.distance;
                this->mass = object.mass;
                this->rotational_velocity = object.rotational_velocity;
                this->member_objects = object.member_objects;
                this->member_number = object.member_number;
            }

            virtual celestial_object& operator=(const celestial_object& object)
            {
                if(&object == this){
                    return *this;
                } else{
                    this->object_type = object.object_type;
                    this->name = object.name;
                    this->redshift = object.redshift;
                    this->distance = object.distance;
                    this->mass = object.mass;
                    this->rotational_velocity = object.rotational_velocity;
                    this->member_objects = object.member_objects;
                    this->member_number = object.member_number;
                    return *this;
                }
            }

            //Move constructor and assignment operator
            celestial_object(celestial_object&& object)
            {
                std::swap(this->object_type, object.object_type);
                std::swap(this->name, object.name);
                std::swap(this->redshift, object.redshift);
                std::swap(this->distance, object.distance);
                std::swap(this->mass, object.mass);
                std::swap(this->rotational_velocity, object.rotational_velocity);
                std::swap(this->member_objects, object.member_objects);
                std::swap(this->member_number, object.member_number);
            }

            virtual celestial_object& operator=(celestial_object&& object)
            {
                std::swap(this->object_type, object.object_type);
                std::swap(this->name, object.name);
                std::swap(this->redshift, object.redshift);
                std::swap(this->distance, object.distance);
                std::swap(this->mass, object.mass);
                std::swap(this->rotational_velocity, object.rotational_velocity);
                std::swap(this->member_objects, object.member_objects);
                std::swap(this->member_number, object.member_number);
                return *this;
            }

            //Raw pointers are not used in favour of smart pointers and weak pointers have been used to prevent cyclical references
            //Hence, there is no special behaviour the destructor needs to perform that is not handled by the smart pointer memory manager
            ~celestial_object() = default;

            void add_member(std::shared_ptr<celestial_object> member_ptr);
            void add_member(std::shared_ptr<celestial_object> member_ptr, double orb_distance, double orb_tilt, double orb_eccentricity);
            //void remove_member();
            //void remove_member(int& index);
            virtual void export_to_file(std::fstream& object_dat, std::fstream& relation_dat);
            celestial_objects::satellite get_member(int& index);
            std::string get_name()const{return name;}
            void get_properties();
            celestial_objects::celestial_types get_type(){return object_type;}
            int get_member_number(){return member_number;}
    
            //Allows for specific properties to be returned to the console, but must be overridden in derived classes.
            //This also represents a convenient function to set as purely virtual, hence making this class abstract.
            virtual void get_additional_properties() = 0;
            std::vector<satellite> get_all_members();

            //Allows the import process to directly access these objects when parsing data
            //friend void catalogue::import_from_file();
    };

    class galaxy : public celestial_object
    {
        /* Represents a galaxy, as with the third assignment in this module, but adapted to work with inheritance from an abstract class.
        This class contains the specific additional information required for galaxies: the stellar mass fraction and the Hubble type.
        Satellite galaxies are handled by the general inherited member_object vector. */
        private:
            double stellar_mass_fraction{0};
            hubble_types hubble_type{hubble_types::Unassigned};
        
        public:
            galaxy():celestial_object()
            {
                object_type = celestial_types::Galaxy;
            }

            galaxy(std::string name_input):celestial_object(name_input)
            {
                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                std::cout << "Enter the galaxy's stellar mass fraction (up to 0.1): ";
                std::cin >> stellar_mass_fraction;
                std::cout<< std::endl;
                while(std::cin.fail() || (stellar_mass_fraction < 0 || stellar_mass_fraction > 0.1)){
                    std::cin.clear();
                    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                    std::cout << "Please type a valid mass fraction: ";
                    std::cin >> stellar_mass_fraction;
                    std::cout<< std::endl;
                }

                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                std::string input_str;
                std::cout << "Enter the galaxy's Hubble type: ";
                std::getline(std::cin, input_str);
                std::cout<< std::endl;
                std::vector<std::string>::const_iterator string_location{std::find(hubble_types_output.begin(),
                hubble_types_output.end(), input_str)};
                while(string_location == hubble_types_output.end()){
                    std::cin.clear();
                    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                    std::cout << "Please enter a valid Hubble type: ";
                    std::getline(std::cin, input_str);
                    std::cout<< std::endl;
                    std::vector<std::string>::const_iterator string_location{std::find(hubble_types_output.begin(),
                    hubble_types_output.end(), input_str)};
                }
                hubble_type = hubble_types(int{string_location - hubble_types_output.begin()});
                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                object_type = celestial_types::Galaxy;
            }

            galaxy(const galaxy& g):celestial_object(g)
            {
                this->object_type = celestial_types::Galaxy;
                this->stellar_mass_fraction = g.stellar_mass_fraction;
                this->hubble_type = g.hubble_type;
            }
            
            galaxy& operator=(const galaxy& g)
            {
                if(&g == this){
                    return *this;
                } else{
                    this->object_type = g.object_type;
                    this->name = g.name;
                    this->redshift = g.redshift;
                    this->distance = g.distance;
                    this->mass = g.mass;
                    this->rotational_velocity = g.rotational_velocity;
                    this->member_objects = g.member_objects;
                    this->member_number = g.member_number;
                    this->stellar_mass_fraction = g.stellar_mass_fraction;
                    this->hubble_type = g.hubble_type;
                    return *this;
                }
            }

            galaxy(galaxy&& g):celestial_object(g)
            {
                this->object_type = celestial_types::Galaxy;
                std::swap(this->stellar_mass_fraction, g.stellar_mass_fraction);
                std::swap(this->hubble_type, g.hubble_type);
            }

            galaxy& operator=(galaxy&& g)
            {
                std::swap(this->object_type, g.object_type);
                std::swap(this->name, g.name);
                std::swap(this->redshift, g.redshift);
                std::swap(this->distance, g.distance);
                std::swap(this->mass, g.mass);
                std::swap(this->rotational_velocity, g.rotational_velocity);
                std::swap(this->member_objects, g.member_objects);
                std::swap(this->member_number, g.member_number);
                std::swap(this->stellar_mass_fraction, g.stellar_mass_fraction);
                std::swap(this->hubble_type, g.hubble_type);
                return *this;
            }

            galaxy(std::string name_input, double z, double dist, double m, double omega, double mass_frac, hubble_types h_type):
            celestial_object(name_input, z, dist, m, omega)
            {
                object_type = celestial_types::Galaxy;
                stellar_mass_fraction = mass_frac;
                hubble_type = h_type;
            }

            void export_to_file(std::fstream& object_dat, std::fstream& relation_dat);
            virtual void get_additional_properties() override;
    };

    class star : public celestial_object
    {
        /* Defines any star or star-derived/non-galactic luminous object. Includes the important data for luminous objects:
        stellar type & digit, luminosity class & value and the relevant magnitudes. */
        protected:
            stellar_types star_type{stellar_types::Unassigned};
            int stellar_digit{0};
            luminosity_class luminosity_id{luminosity_class::Unassigned};
            double abs_magnitude{0};
            double app_magnitude{0};
        
        public:
            star():celestial_object()
            {
                object_type = celestial_types::Star;
            }

            star(std::string name_input):celestial_object(name_input)
            {
                //Add Stelalr digit stuff
                std::string input_str;
                std::cout << "Enter the star's stellar classification: ";
                std::getline(std::cin, input_str);
                std::cout << std::endl;
                std::vector<std::string>::const_iterator string_location{std::find(stellar_types_output.begin(),
                stellar_types_output.end(), input_str)};
                while(string_location == stellar_types_output.end()){
                    std::cin.clear();
                    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                    std::cout << "Please enter a valid classification: ";
                    std::getline(std::cin, input_str);
                    std::vector<std::string>::const_iterator string_location{std::find(stellar_types_output.begin(),
                    stellar_types_output.end(), input_str)};
                }
                star_type = stellar_types(int{string_location - stellar_types_output.begin()});
                
                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                std::cout << "Enter the star's stellar classification digit (0-9): ";
                std::cin >> stellar_digit;
                std::cout << std::endl;
                while(std::cin.fail() || stellar_digit < 0 || stellar_digit > 10){
                    std::cout << "Please enter a valid digit: ";
                    std::cin >> stellar_digit;
                    std::cout << std::endl;
                }

                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                object_type = celestial_types::Star;
            }

            star(const star& s):celestial_object(s)
            {
                object_type = celestial_types::Star;
                this->star_type = s.star_type;
                this->stellar_digit = s.stellar_digit;
                this->luminosity_id = s.luminosity_id;
                this->abs_magnitude = s.abs_magnitude;
                this->app_magnitude = s.app_magnitude;
            }

            star& operator=(const star& s)
            {
                if(&s == this){
                    return *this;
                } else{
                    this->object_type = s.object_type;
                    this->name = s.name;
                    this->redshift = s.redshift;
                    this->distance = s.distance;
                    this->mass = s.mass;
                    this->rotational_velocity = s.rotational_velocity;
                    this->member_objects = s.member_objects;
                    this->member_number = s.member_number;
                    this->star_type = s.star_type;
                    this->stellar_digit = s.stellar_digit;
                    this->luminosity_id = s.luminosity_id;
                    this->abs_magnitude = s.abs_magnitude;
                    this->app_magnitude = s.app_magnitude;
                    return *this;
                }
            }

            star(star&& s):celestial_object(s)
            {
                object_type = celestial_types::Star;
                std::swap(this->star_type, s.star_type);
                std::swap(this->stellar_digit, s.stellar_digit);
                std::swap(this->luminosity_id, s.luminosity_id);
                std::swap(this->abs_magnitude, s.abs_magnitude);
                std::swap(this->app_magnitude, s.app_magnitude);
            }

            star& operator=(star&& s)
            {
                std::swap(this->object_type, s.object_type);
                std::swap(this->name, s.name);
                std::swap(this->redshift, s.redshift);
                std::swap(this->distance, s.distance);
                std::swap(this->mass, s.mass);
                std::swap(this->rotational_velocity, s.rotational_velocity);
                std::swap(this->member_objects, s.member_objects);
                std::swap(this->member_number, s.member_number);
                std::swap(this->star_type, s.star_type);
                std::swap(this->stellar_digit, s.stellar_digit);
                std::swap(this->luminosity_id, s.luminosity_id);
                std::swap(this->abs_magnitude, s.abs_magnitude);
                std::swap(this->app_magnitude, s.app_magnitude);
                return *this;
            }

            star(std::string name_input, double z, double dist, double m, double omega, stellar_types s_type,
            int s_digit, luminosity_class lum_no, double abs_mag, double app_mag):celestial_object(name_input, z,
            dist, m, omega)
            {
                object_type = celestial_types::Star;
                star_type = s_type;
                stellar_digit = s_digit;
                luminosity_id = lum_no;
                abs_magnitude = abs_mag;
                app_magnitude = app_mag;
            }

            void export_to_file(std::fstream& object_dat, std::fstream& relation_dat);
            virtual void get_additional_properties() override;
    };

    class main_sequence_star : public star
    {
        /* Represents any main sequence star, mainly for easier identification. */
        public:
            main_sequence_star():star()
            {
                object_type = celestial_types::MainSequenceStar;
            }

            main_sequence_star(std::string name_input):star(name_input){object_type = celestial_types::MainSequenceStar;}
            main_sequence_star(const main_sequence_star& msstar):star(msstar){object_type = celestial_types::MainSequenceStar;}
            main_sequence_star(main_sequence_star&& msstar):star(msstar){object_type = celestial_types::MainSequenceStar;}

            main_sequence_star(std::string name_input, double z, double dist, double m, double omega, stellar_types s_type,
            int s_digit, luminosity_class lum_no, double abs_mag, double app_mag):star(name_input, z, dist, m, omega, 
            s_type, s_digit, lum_no, abs_mag, app_mag)
            {
                object_type = celestial_types::MainSequenceStar;
            }
    };

    class red_giant_star : public star
    {
        /* Represents any red giant star, as they are still classified as stars despite being remnants and are
        generally less uniform than other types of remnants. */
        public:
            red_giant_star():star()
            {
                object_type = celestial_types::RedGiantStar;
            }

            red_giant_star(std::string name_input):star(name_input){object_type = celestial_types::RedGiantStar;}
            red_giant_star(const red_giant_star& rgstar):star(rgstar){object_type = celestial_types::RedGiantStar;}
            red_giant_star(red_giant_star&& rgstar):star(rgstar){object_type = celestial_types::RedGiantStar;};

            red_giant_star(std::string name_input, double z, double dist, double m, double omega, stellar_types s_type,
            int s_digit, luminosity_class lum_no, double abs_mag, double app_mag):star(name_input, z, dist, m, omega, 
            s_type, s_digit, lum_no, abs_mag, app_mag)
            {
                object_type = celestial_types::RedGiantStar;
            }
    };

    class planet : public celestial_object
    {
        /* Represents a planet within a stellar system, or a wandering planet when not contained by in a stellar system. 
        Can be a derived class if the planet type is unknown, but also has specialisations to terrestrial, gaseous and
        dwarf planets. */
        public:
            planet():celestial_object()
            {
                object_type = celestial_types::Planet;
            }

            planet(std::string name_input):celestial_object(name_input){object_type = celestial_types::Planet;}
            planet(const planet& plan):celestial_object(plan){object_type = celestial_types::Planet;};
            planet(planet&& plan):celestial_object(plan){object_type = celestial_types::Planet;};

            planet(std::string name_input, double z, double dist, double m, double omega):celestial_object(name_input, z, dist, m, omega)
            {
                object_type = celestial_types::Planet;
            }

            virtual void get_additional_properties() override
            {
                std::cout << "No additional properties. " << std::endl;
            }
    };

    class terrestrial_planet : public planet
    {
        /* Represents a terrestrial planet (anything with a substantial mass of rock/ice/water). */
        public:
            terrestrial_planet():planet()
            {
                object_type = celestial_types::TerrestrialPlanet;
            }

            terrestrial_planet(std::string name_input):planet(name_input){object_type = celestial_types::TerrestrialPlanet;}
            terrestrial_planet(const terrestrial_planet& t_plan):planet(t_plan){object_type = celestial_types::TerrestrialPlanet;};
            terrestrial_planet(terrestrial_planet&& t_plan):planet(t_plan){object_type = celestial_types::TerrestrialPlanet;};

            terrestrial_planet(std::string name_input, double z, double dist, double m, double omega):planet(name_input, z, dist, m, omega)
            {
                object_type = celestial_types::TerrestrialPlanet;
            }
    };

    class gaseous_planet : public planet
    {
        /* Represents a gas giant planet. */
        public:
            gaseous_planet():planet()
            {
                object_type = celestial_types::GaseousPlanet;
            }

            gaseous_planet(std::string name_input):planet(name_input){object_type = celestial_types::GaseousPlanet;}
            gaseous_planet(const gaseous_planet& t_plan):planet(t_plan){object_type = celestial_types::GaseousPlanet;}
            gaseous_planet(gaseous_planet&& t_plan):planet(t_plan){object_type = celestial_types::GaseousPlanet;}

            gaseous_planet(std::string name_input, double z, double dist, double m, double omega):planet(name_input, z, dist, m, omega)
            {
                object_type = celestial_types::GaseousPlanet;
            }
    };

    class dwarf_planet : public planet
    {
        /* Special case for notable dwarf planets, otherwise they would fall under the asteroid class. */
        public:
            dwarf_planet():planet()
            {
                object_type = celestial_types::DwarfPlanet;
            }

            dwarf_planet(std::string name_input):planet(name_input){object_type = celestial_types::DwarfPlanet;}
            dwarf_planet(const dwarf_planet& d_plan):planet(d_plan){object_type = celestial_types::DwarfPlanet;}
            dwarf_planet(dwarf_planet&& d_plan):planet(d_plan){object_type = celestial_types::DwarfPlanet;}

            dwarf_planet(std::string name_input, double z, double dist, double m, double omega):planet(name_input, z, dist, m, omega)
            {
                object_type = celestial_types::DwarfPlanet;
            }
    };

    class moon : public celestial_object
    {
        /* Represents a moon of a planet. */
        public:
            moon():celestial_object()
            {
                object_type = celestial_types::Moon;
            }
            
            moon(std::string name_input):celestial_object(name_input){object_type = celestial_types::Moon;}
            moon(const moon& m):celestial_object(m){object_type = celestial_types::Moon;}
            moon(moon&& m):celestial_object(m){object_type = celestial_types::Moon;}

            moon(std::string name_input, double z, double dist, double m, double omega):celestial_object(name_input, z, dist, m, omega)
            {
                object_type = celestial_types::Moon;
            }

            virtual void get_additional_properties() override
            {
                std::cout << "No additional properties. " << std::endl;
            }
    };

    class comet : public celestial_object
    {
        /* Represents a comet within a stellar system. */
        public:
            comet():celestial_object()
            {
                object_type = celestial_types::Comet;
            }

            comet(std::string name_input):celestial_object(name_input){object_type = celestial_types::Comet;}
            comet(const comet& c):celestial_object(c){object_type = celestial_types::Comet;}
            comet(comet&& c):celestial_object(c){object_type = celestial_types::Comet;}

            comet(std::string name_input, double z, double dist, double m, double omega):celestial_object(name_input, z, dist, m, omega)
            {
                object_type = celestial_types::Comet;
            }

            virtual void get_additional_properties() override
            {
                std::cout << "No additional properties. " << std::endl;
            }
    };

    class asteroid : public celestial_object
    {
        /* Represents an asteroid or meteorite withing a stellar system. */
        public:
            asteroid():celestial_object()
            {
                object_type = celestial_types::Asteroid;
            }

            asteroid(std::string name_input):celestial_object(name_input){object_type = celestial_types::Asteroid;}
            asteroid(const asteroid& aster):celestial_object(aster){object_type = celestial_types::Asteroid;}
            asteroid(asteroid&& aster):celestial_object(aster){object_type = celestial_types::Asteroid;}

            asteroid(std::string name_input, double z, double dist, double m, double omega):celestial_object(name_input, z, dist, m, omega)
            {
                object_type = celestial_types::Asteroid;
            }

            virtual void get_additional_properties() override
            {
                std::cout << "No additional properties. " << std::endl;
            }
    };

    class satellite
    { 
        /* Acts as a container for celestial objects which are gravitationally bound to others. Includes the 
        important orbital parameters (semi-major axis, tilt and eccentricity) and a pointer to the object contained.
        Otherwise, a satellite object acts as if it is the object it contains. May also act as an object representing
        a man-made satellite. */
        protected:
            double orbit_distance{1};
            double orbit_tilt{0};
            double orbit_eccentricity{1};
            std::weak_ptr<celestial_object> satellite_object;

        public:
            //Discovered that I need to declare a class as a friend first before declaring its functions as friends
            friend class celestial_object;
            friend class galaxy;
            friend class star;

            satellite() = default;

            satellite(std::shared_ptr<celestial_object> sat_object)
            {
                satellite_object = std::weak_ptr<celestial_object>(sat_object);

                std::cout << "Enter the distance of the orbit (in pc): ";
                std::cin >> orbit_distance;
                std::cout<< std::endl;
                while(std::cin.fail() || (orbit_distance < 0)){
                    std::cin.clear();
                    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                    std::cout << "Please enter a valid orbit distance : ";
                    std::cin >> orbit_distance;
                    std::cout<< std::endl;
                }
                
                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

                std::cout << "Enter the tilt of the orbit (in degrees, between -180 and 180): ";
                std::cin >> orbit_tilt;
                std::cout<< std::endl;
                while(std::cin.fail() || (orbit_tilt < -180 || orbit_tilt > 180)){
                    std::cin.clear();
                    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                    std::cout << "Please enter a valid tilt angle: ";
                    std::cin >> orbit_tilt;
                    std::cout<< std::endl;
                }
                
                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

                std::cout << "Enter the eccentricity of the orbit (>= 0): ";
                std::cin >> orbit_eccentricity;
                std::cout<< std::endl;
                while(std::cin.fail() || (orbit_eccentricity < 0)){
                    std::cin.clear();
                    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                    std::cout << "Please enter a valid eccentricity: ";
                    std::cin >> orbit_eccentricity;
                    std::cout<< std::endl;
                }
                
                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            }

            satellite(std::shared_ptr<celestial_object> object_ptr, double orb_dist, double orb_tilt, double orb_ecc)
            {
                orbit_distance = orb_dist;
                orbit_tilt = orb_tilt;
                orbit_eccentricity = orb_ecc;
                satellite_object = std::weak_ptr<celestial_object>(object_ptr);
            }

            //friend void catalogue::export_to_file();
            friend void celestial_object::export_to_file(std::fstream& object_dat, std::fstream& relation_dat);
            friend void galaxy::export_to_file(std::fstream& object_dat, std::fstream& relation_dat);
            friend void star::export_to_file(std::fstream& object_dat, std::fstream& relation_dat);

            satellite(const satellite& sat)
            {
                this->orbit_distance = sat.orbit_distance;
                this->orbit_tilt = sat.orbit_tilt;
                this->orbit_eccentricity = sat.orbit_tilt;
                this->satellite_object = sat.satellite_object;
            }

            satellite& operator=(const satellite& sat)
            {
                if(&sat == this){
                    return *this;
                } else{
                    this->orbit_distance = sat.orbit_distance;
                    this->orbit_tilt = sat.orbit_tilt;
                    this->orbit_eccentricity = sat.orbit_tilt;
                    this->satellite_object = sat.satellite_object;
                    return *this;
                }
            }

            satellite(satellite&& sat)
            {
                std::swap(this->orbit_distance, sat.orbit_distance);
                std::swap(this->orbit_tilt, sat.orbit_tilt);
                std::swap(this->orbit_eccentricity, sat.orbit_eccentricity);
                std::swap(this->satellite_object, sat.satellite_object);
            }

            satellite& operator=(satellite&& sat)
            {
                std::swap(this->orbit_distance, sat.orbit_distance);
                std::swap(this->orbit_tilt, sat.orbit_tilt);
                std::swap(this->orbit_eccentricity, sat.orbit_eccentricity);
                std::swap(this->satellite_object, sat.satellite_object);
                return *this;
            }

            ~satellite() = default;

            std::shared_ptr<celestial_object> get_object();
    };

    class stellar_remnant : public star
    {
        /* Derived & base class representing anything left over after a star exits its main sequence. This can be used
        as an object itself to represent a nebula/gas bubble, but also has specialisations to supernovae,
        neutron stars and black holes.*/
        public:
            stellar_remnant():star()
            {
                object_type = celestial_types::StellarRemnant;
            }

            stellar_remnant(std::string name_input):star(name_input){object_type = celestial_types::StellarRemnant;}
            stellar_remnant(const stellar_remnant& strem):star(strem){object_type = celestial_types::StellarRemnant;}
            stellar_remnant(stellar_remnant&& strem):star(strem){object_type = celestial_types::StellarRemnant;}

            stellar_remnant(std::string name_input, double z, double dist, double m, double omega, stellar_types s_type,
            int s_digit, luminosity_class lum_no, double abs_mag, double app_mag):star(name_input, z, dist, m, omega,
            s_type, s_digit, lum_no, abs_mag, app_mag)
            {
                object_type = celestial_types::StellarRemnant;
            }
    };

    class supernova : public stellar_remnant
    {
        /* Represents an observed supernova, especially useful for tracking standard candles. */
        public:
            supernova():stellar_remnant()
            {
                object_type = celestial_types::Supernova;
            }

            supernova(std::string name_input):stellar_remnant(name_input){object_type = celestial_types::Supernova;}
            supernova(const supernova& nova):stellar_remnant(nova){object_type = celestial_types::Supernova;}
            supernova(supernova&& nova):stellar_remnant(nova){object_type = celestial_types::Supernova;}

            supernova(std::string name_input, double z, double dist, double m, double omega, stellar_types s_type,
            int s_digit, luminosity_class lum_no, double abs_mag, double app_mag):stellar_remnant(name_input, z, dist, m, omega,
            s_type, s_digit, lum_no, abs_mag, app_mag)
            {
                object_type = celestial_types::Supernova;
            }
    };

    class neutron_star : public stellar_remnant
    {
        /* Represents a neutron star, also useful for tracking standard candles. */
        public:
            neutron_star():stellar_remnant()
            {
                object_type = celestial_types::NeutronStar;
            }

            neutron_star(std::string name_input):stellar_remnant(name_input){object_type = celestial_types::NeutronStar;}
            neutron_star(const neutron_star& nstar):stellar_remnant(nstar){object_type = celestial_types::NeutronStar;}
            neutron_star(neutron_star&& nstar):stellar_remnant(nstar){object_type = celestial_types::NeutronStar;}

            neutron_star(std::string name_input, double z, double dist, double m, double omega, stellar_types s_type,
            int s_digit, luminosity_class lum_no, double abs_mag, double app_mag):stellar_remnant(name_input, z, dist, m, omega,
            s_type, s_digit, lum_no, abs_mag, app_mag)
            {
                object_type = celestial_types::NeutronStar;
            }
    };

    class pulsar : public neutron_star
    {
        /* Represents a pulsar, so it makes sense to inherit from the neutron_star class, given that pulsars are a specific subset
        of neutron stars. */
        public:
            pulsar():neutron_star()
            {
                object_type = celestial_types::Pulsar;
            }

            pulsar(std::string name_input):neutron_star(name_input){object_type = celestial_types::Pulsar;}
            pulsar(const pulsar& puls):neutron_star(puls){object_type = celestial_types::Pulsar;}
            pulsar(pulsar&& puls):neutron_star(puls){object_type = celestial_types::Pulsar;}

            pulsar(std::string& name_input, double& z, double& dist, double& m, double omega, stellar_types s_type,
            int s_digit, luminosity_class lum_no, double abs_mag, double app_mag):neutron_star(name_input, z, dist, m, omega,
            s_type, s_digit, lum_no, abs_mag, app_mag)
            {
                object_type = celestial_types::Pulsar;
            }
    };

    class black_hole : public celestial_object
    {
        /* Represents a black hole. Although this is a stellar remnant, it only inherits from the base celestial_object
        class as there are many members that other celestial remnants have that black holes don't, notably inherent luminosity
        and classification on the Hertzsprung-Russel diagram. */
        public:
            black_hole():celestial_object()
            {
                object_type = celestial_types::BlackHole;
            }

            black_hole(std::string name_input):celestial_object(name_input){object_type = celestial_types::BlackHole;}
            black_hole(const black_hole& bh):celestial_object(bh){object_type = celestial_types::BlackHole;}
            black_hole(black_hole&& bh):celestial_object(bh){object_type = celestial_types::BlackHole;}

            black_hole(std::string name_input, double z, double dist, double m, double omega):celestial_object(name_input, z, dist, m, omega)
            {
                object_type = celestial_types::BlackHole;
            }

            virtual void get_additional_properties() override
            {
                std::cout << "No additional properties. " << std::endl;
            }
    }; 

    class catalogue
    {
        /* Acts as a container for all celestial objects in a given collection. */
        private:
            std::string catalogue_name{""};
            std::vector<std::shared_ptr<celestial_object>> catalogue_objects{};
            std::vector<std::string> local_object_names{};
            int object_amount{0};
            std::vector<std::shared_ptr<celestial_object>>::iterator catalogue_begin{catalogue_objects.begin()};
            std::vector<std::shared_ptr<celestial_object>>::iterator catalogue_end{catalogue_objects.end()};
            std::vector<std::shared_ptr<celestial_object>>::iterator catalogue_position{catalogue_objects.begin()};
            
        public:
            catalogue()
            {
                import_from_file();
            }

            catalogue(std::string name)
            {
                catalogue_name = name;
            }

            catalogue(const catalogue& cat)
            {
                this->catalogue_name = cat.catalogue_name;
                this->catalogue_objects = cat.catalogue_objects;
                this->local_object_names = cat.local_object_names;
                this->object_amount = cat.object_amount;
                this->catalogue_begin = this->catalogue_objects.begin();
                this->catalogue_end = this->catalogue_objects.end();
                this->catalogue_position = this->catalogue_objects.begin();
            }

            catalogue& operator=(const catalogue& cat)
            {
                if(&cat == this){
                    return *this;
                } else{
                    this->catalogue_name = cat.catalogue_name;
                    this->catalogue_objects = cat.catalogue_objects;
                    this->local_object_names = cat.local_object_names;
                    this->object_amount = cat.object_amount;
                    this->catalogue_begin = this->catalogue_objects.begin();
                    this->catalogue_end = this->catalogue_objects.end();
                    this->catalogue_position = this->catalogue_objects.begin();
                    return *this;
                }
            }

            catalogue(catalogue&& cat)
            {
                std::swap(this->catalogue_name, cat.catalogue_name);
                std::swap(this->catalogue_objects, cat.catalogue_objects);
                std::swap(this->local_object_names, cat.local_object_names);
                std::swap(this->object_amount, cat.object_amount);
                this->catalogue_begin = this->catalogue_objects.begin();
                this->catalogue_end = this->catalogue_objects.end();
                this->catalogue_position = this->catalogue_objects.begin();
            }

            catalogue& operator=(catalogue&& cat)
            {
                std::swap(this->catalogue_name, cat.catalogue_name);
                std::swap(this->catalogue_objects, cat.catalogue_objects);
                std::swap(this->local_object_names, cat.local_object_names);
                std::swap(this->object_amount, cat.object_amount);
                this->catalogue_begin = this->catalogue_objects.begin();
                this->catalogue_end = this->catalogue_objects.end();
                this->catalogue_position = this->catalogue_objects.begin();
                return *this;                
            }

            ~catalogue() = default;
            std::string get_name();
            std::vector<std::string> get_obj_names(){return local_object_names;}
            std::shared_ptr<celestial_object> get_object(std::string name);
            std::shared_ptr<celestial_object> get_object(int index);
            void push_obj_name(std::string name){local_object_names.push_back(name);}
            int get_number(){return object_amount;}
            void import_from_file();
            void export_to_file();
            void add_object(celestial_object* object);
            //void remove_object();
            void sort_catalogue(parameters& parameter);
            std::vector<std::shared_ptr<celestial_object>> subselect_catalogue(celestial_types& type);
            void generate_report();

    };

    bool name_sort(std::string name_a, std::string name_b);
    bool numerical_sort(int& a, int& b);
    bool numerical_sort(double& a, double& b);
    bool hubble_sort(celestial_objects::hubble_types& hubtype_a, celestial_objects::hubble_types& hubtype_b);
    bool stellar_sort(celestial_objects::stellar_types& steltype_a, celestial_objects::stellar_types& steltype_b);
}

#endif
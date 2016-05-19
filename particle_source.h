#ifndef _PARTICLE_SOURCE_H_
#define _PARTICLE_SOURCE_H_

#include <cmath>
#include <random>
#include <iostream>
#include <iomanip>
#include <vector>
#include <hdf5.h>
#include <hdf5_hl.h>
#include <mpi.h>
#include "config.h"
#include "particle.h"
#include "vec3d.h"

class Particle_source{
public:
    std::string name;
    std::vector<Particle> particles;
private:
    int initial_number_of_particles;
    int particles_to_generate_each_step;
    unsigned int max_id;
    // Source position
    double xleft;
    double xright;
    double ytop;
    double ybottom;
    double znear;
    double zfar;
    // Momentum
    Vec3d mean_momentum;
    double temperature;
    // Particle characteristics
    double charge;
    double mass;
    // Random number generator
    std::default_random_engine rnd_gen;
public:
    Particle_source( Config &conf, Source_config_part &src_conf );
    void generate_each_step();
    void update_particles_position( double dt );	
    void print_particles();
    void write_to_file( hid_t hdf5_file_id );
    virtual ~Particle_source() {};
private:
    // Particle initialization
    void set_parameters_from_config( Source_config_part &src_conf );
    void generate_initial_particles();
    // Todo: replace 'std::default_random_engine' type with something more general.
    void generate_num_of_particles( int num_of_particles );
    int num_of_particles_for_each_process( int num_of_particles );
    void populate_vec_of_ids( std::vector<int> &vec_of_ids,
			      int num_of_particles_for_this_proc );
    //int generate_particle_id( const int number, const int proc );
    Vec3d uniform_position_in_cube( const double xleft, const double ytop,
				    const double znear, const double xright,
				    const double ybottom, const double zfar,
				    std::default_random_engine &rnd_gen );
    double random_in_range( const double low, const double up,
			    std::default_random_engine &rnd_gen );
    Vec3d maxwell_momentum_distr( const Vec3d mean_momentum,
				  const double temperature, const double mass, 
				  std::default_random_engine &rnd_gen );
    // Check config
    void check_correctness_of_related_config_fields( 
	Config &conf, Source_config_part &src_conf );
    void particle_source_initial_number_of_particles_gt_zero( 
	Config &conf, Source_config_part &src_conf );
    void particle_source_particles_to_generate_each_step_ge_zero( 
	Config &conf, Source_config_part &src_conf );
    void particle_source_x_left_ge_zero( 
	Config &conf, Source_config_part &src_conf );
    void particle_source_x_left_le_particle_source_x_right( 
	Config &conf, Source_config_part &src_conf );
    void particle_source_x_right_le_grid_x_size( 
	Config &conf, Source_config_part &src_conf );
    void particle_source_y_bottom_ge_zero( 
	Config &conf, Source_config_part &src_conf );
    void particle_source_y_bottom_le_particle_source_y_top( 
	Config &conf, Source_config_part &src_conf );
    void particle_source_y_top_le_grid_y_size( 
	Config &conf, Source_config_part &src_conf );
    void particle_source_z_near_ge_zero( 
	Config &conf, Source_config_part &src_conf );
    void particle_source_z_near_le_particle_source_z_far( 
	Config &conf, Source_config_part &src_conf );
    void particle_source_z_far_le_grid_z_size( 
	Config &conf, Source_config_part &src_conf );
    void particle_source_temperature_gt_zero( 
	Config &conf, Source_config_part &src_conf );
    void particle_source_mass_gt_zero( 
	Config &conf, Source_config_part &src_conf );
    // Write to file
    void write_hdf5_particles( hid_t current_source_group_id );
    void write_hdf5_source_parameters( hid_t current_source_group_id );
    void hdf5_status_check( herr_t status );
    int total_particles_across_all_processes();
    int data_offset_for_each_process_for_1d_dataset();
};


class Particle_sources_manager{
public:
    std::vector<Particle_source> sources;
public:
    Particle_sources_manager( Config &conf );
    virtual ~Particle_sources_manager() {};
    void write_to_file( hid_t hdf5_file_id )
    {
	hid_t group_id;
	herr_t status;
	int single_element = 1;
	std::string hdf5_groupname = "/Particle_sources";
	int n_of_sources = sources.size();
	group_id = H5Gcreate2( hdf5_file_id, hdf5_groupname.c_str(),
			       H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
	hdf5_status_check( group_id );

	status = H5LTset_attribute_int( hdf5_file_id, hdf5_groupname.c_str(),
			       "number_of_sources", &n_of_sources, single_element );
	hdf5_status_check( status );
	
	for( auto &src : sources )
	    src.write_to_file( group_id );

	status = H5Gclose( group_id );
	hdf5_status_check( status );
    }; 
    void generate_each_step()
    {
	for( auto &src : sources )
	    src.generate_each_step();
    };
    void print_particles()
    {
	for( auto &src : sources )
	    src.print_particles();
    };
    void update_particles_position( double dt )
    {
	for( auto &src : sources )
	    src.update_particles_position( dt );
    };
    void hdf5_status_check( herr_t status )
    {
	if( status < 0 ){
	    std::cout << "Something went wrong while writing"
		      << "'Particle_sources' group. Aborting."
		      << std::endl;
	    exit( EXIT_FAILURE );
	}
    };
};


#endif /* _PARTICLE_SOURCE_H_ */

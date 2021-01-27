#pragma once

#include "EigenInclude.h"
#include "types.h"
#include "biotsavart.h"
#include "unitsourcedensity.h"

#include <fstream>

namespace UVLM
{
    namespace Matrix
    {
        // DECLARATIONS
        template <typename t_zeta,
                  typename t_zeta_col,
                  typename t_zeta_star,
                //   typename t_zeta_star_col,
                  typename t_uext_col,
                  typename t_normals,
                  typename t_aic>
        void AIC
        (
            const uint& Ktotal,
            const t_zeta& zeta,
            const t_zeta_col& zeta_col,
            const t_zeta_star& zeta_star,
            // const t_zeta_star_col& zeta_star_col,
            const t_uext_col& uext_col,
            const t_normals& normals,
            const UVLM::Types::VMopts& options,
            const bool horseshoe,
            t_aic& aic
        );

        template <typename t_zeta,
                  typename t_zeta_col,
                  typename t_uext_col,
                  typename t_surf_vec_panel,
                  typename t_surf_vec_col,
                  typename t_u_induced_col,
                  typename t_aic>
        void AIC_sources
        (
            const uint& Ktotal,
            const t_zeta& zeta,
            const t_zeta_col& zeta_col,
            const t_uext_col& uext_col,
            const t_surf_vec_panel& longitudinals_panel,
            const t_surf_vec_panel& perpendiculars_panel,
            const t_surf_vec_panel& normals_panel,
            const t_surf_vec_col& longitudinals_col,
            const t_surf_vec_col& perpendiculars_col,
            const t_surf_vec_col& normals_col,
            const UVLM::Types::VMopts& options,
	        t_u_induced_col& u_induced_col_x,
	        t_u_induced_col& u_induced_col_y,
	        t_u_induced_col& u_induced_col_z,
            t_aic& aic_sources
        );

        template <typename t_zeta_col,
                  typename t_zeta_star,
                  typename t_uext_col,
                //   typename t_zeta_dot_col,
                  typename t_gamma_star,
                  typename t_normal>
        void RHS
        (
            const t_zeta_col& zeta_col,
            const t_zeta_star& zeta_star,
            const t_uext_col& uext_col,
            // const t_zeta_dot_col& zeta_dot_col,
            const t_gamma_star& gamma_star,
            const t_normal& normal,
            const UVLM::Types::VMopts& options,
            UVLM::Types::VectorX& rhs,
            const uint& Ktotal
        );


        template <typename t_uext_col,
                  typename t_normal>
        void RHS_nonlifting_body
        (
            const t_uext_col& uinc_col,
            const t_normal& normal,
            UVLM::Types::VectorX& rhs,
            const uint& Ktotal,
            const uint n_surf
        );


        void generate_assembly_offset
        (
            const UVLM::Types::VecDimensions& dimensions,
            const UVLM::Types::VecDimensions& dimensions_star,
            UVLM::Types::MatrixX& offset,
            const bool steady
        );


        template <typename t_gamma,
                  typename t_zeta_col>
        void reconstruct_gamma
        (
            const UVLM::Types::VectorX& gamma_flat,
            t_gamma& gamma,
            const t_zeta_col& zeta_col
        );


        template <typename t_gamma,
                  typename t_zeta_col>
        void deconstruct_gamma
        (
            const t_gamma& gamma,
            UVLM::Types::VectorX& gamma_flat,
            const t_zeta_col& zeta_col
        );
		
		template <typename t_vec_mat,
		          typename t_zeta_col>
		void reconstruct_MatrixX
		(
			const UVLM::Types::MatrixX& mat_flat,
			t_vec_mat& vec_mat,
			const t_zeta_col& zeta_col
		);
		void build_offsets
		(
			const uint& n_surf,
			const UVLM::Types::VecDimensions& dimensions,
			std::vector<uint>& offset
		);
    }
}
// SOURCE CODE
/*-----------------------------------------------------------------------------

-----------------------------------------------------------------------------*/

template <typename t_zeta,
          typename t_zeta_col,
          typename t_zeta_star,
        //   typename t_zeta_star_col,
          typename t_uext_col,
          typename t_normals,
          typename t_aic>
void UVLM::Matrix::AIC
(
    const uint& Ktotal,
    const t_zeta& zeta,
    const t_zeta_col& zeta_col,
    const t_zeta_star& zeta_star,
    // const t_zeta_star_col& zeta_star_col,
    const t_uext_col& uext_col,
    const t_normals& normals,
    const UVLM::Types::VMopts& options,
    const bool horseshoe,
    t_aic& aic
)
{
    const uint n_surf_panel = zeta.size();
    const uint n_surf_col = zeta_col.size();
    UVLM::Types::VecDimensions dimensions_panel, dimensions_col, dimensions_star;
    UVLM::Types::generate_dimensions(zeta, dimensions_panel, - 1);
    UVLM::Types::generate_dimensions(zeta_star, dimensions_star, -1);
    UVLM::Types::generate_dimensions(zeta_col, dimensions_col, 0);

    // build the offsets beforehand
    // (parallel variation)
    std::vector<uint> offset_panel;
    std::vector<uint> offset_col;
	UVLM::Matrix::build_offsets(n_surf_panel,
								dimensions_panel,
								offset_panel);
	UVLM::Matrix::build_offsets(n_surf_col,
								dimensions_col,
								offset_col);

    // fill up AIC
    for (uint icol_surf=0; icol_surf<n_surf_col; ++icol_surf)
    {
        uint k_surf_col = dimensions_col[icol_surf].first*
                          dimensions_col[icol_surf].second;

        // uint ii_offset = 0;
        for (uint jpanel_surf=0; jpanel_surf<n_surf_panel; ++jpanel_surf)
        {
            uint k_surf_panel = dimensions_panel[jpanel_surf].first*
								dimensions_panel[jpanel_surf].second;
            UVLM::Types::MatrixX dummy_gamma;
            UVLM::Types::MatrixX dummy_gamma_star;
            UVLM::Types::Block block = aic.block(offset_col[icol_surf], offset_panel[jpanel_surf], k_surf_col, k_surf_panel);
            // steady wake coefficients
            dummy_gamma.setOnes(dimensions_panel[jpanel_surf].first,
                                dimensions_panel[jpanel_surf].second);
            if (options.Steady)
            {
                dummy_gamma_star.setOnes(dimensions_star[jpanel_surf].first,
                                        dimensions_star[jpanel_surf].second);

                UVLM::BiotSavart::multisurface_steady_wake
                (
                    zeta[jpanel_surf],
                    zeta_star[jpanel_surf],
                    dummy_gamma,
                    dummy_gamma_star,
                    zeta_col[icol_surf],
                    horseshoe,
                    block,
                    options.ImageMethod,
                    normals[icol_surf],
                    options.vortex_radius
                );
            } else // unsteady case
            {
                dummy_gamma_star.setOnes(1,
                                         dimensions_star[jpanel_surf].second);
                UVLM::BiotSavart::multisurface_unsteady_wake
                (
                    zeta[jpanel_surf],
                    zeta_star[jpanel_surf],
                    dummy_gamma,
                    dummy_gamma_star,
                    zeta_col[icol_surf],
                    block,
                    options.ImageMethod,
                    normals[icol_surf],
                    0,
                    options.vortex_radius
                );
            }
        }
    }
}
/*-----------------------------------------------------------------------------

-----------------------------------------------------------------------------*/

void UVLM::Matrix::build_offsets
(
    const uint& n_surf,
    const UVLM::Types::VecDimensions& dimensions,
    std::vector<uint>& offset
)
{
    uint i_offset = 0;
    uint k_surf;
    for (uint i_surf=0; i_surf<n_surf; ++i_surf)
    {
        offset.push_back(i_offset);
        k_surf = dimensions[i_surf].first*
                 dimensions[i_surf].second;
       i_offset += k_surf;
    }
}
/*-----------------------------------------------------------------------------

-----------------------------------------------------------------------------*/

template <typename t_zeta,
          typename t_zeta_col,
          typename t_uext_col,
          typename t_surf_vec_panel,
          typename t_surf_vec_col,
          typename t_u_induced_col,
          typename t_aic>

void UVLM::Matrix::AIC_sources
(
    const uint& Ktotal,
    const t_zeta& zeta,
    const t_zeta_col& zeta_col,
    const t_uext_col& uext_col,
    const t_surf_vec_panel& longitudinals_panel,
    const t_surf_vec_panel& perpendiculars_panel,
    const t_surf_vec_panel& normals_panel,
    const t_surf_vec_col& longitudinals_col,
    const t_surf_vec_col& perpendiculars_col,
    const t_surf_vec_col& normals_col,
    const UVLM::Types::VMopts& options,
	t_u_induced_col& u_induced_col_x,
	t_u_induced_col& u_induced_col_y,
	t_u_induced_col& u_induced_col_z,
    t_aic& aic_sources
)
{
    const uint n_surf_panel = zeta.size();
    const uint n_surf_col = zeta_col.size();
    UVLM::Types::VecDimensions dimensions_panel, dimensions_col;
    UVLM::Types::generate_dimensions(zeta, dimensions_panel, - 1);
    UVLM::Types::generate_dimensions(zeta_col, dimensions_col, 0);


    // build the offsets beforehand
    // (parallel variation)
    std::vector<uint> offset_panel;
    std::vector<uint> offset_col;
	UVLM::Matrix::build_offsets(n_surf_panel,
								dimensions_panel,
								offset_panel);
	UVLM::Matrix::build_offsets(n_surf_col,
								dimensions_col,
								offset_col);

    // fill up AIC
    for (uint icol_surf=0; icol_surf<n_surf_col; ++icol_surf)
    {
        uint k_surf_col_i = dimensions_col[icol_surf].first*
                            dimensions_col[icol_surf].second;

        // uint ii_offset = 0;
        for (uint jpanel_surf=0; jpanel_surf<n_surf_panel; ++jpanel_surf)
        {
            uint k_surf_panel_j = dimensions_panel[jpanel_surf].first*
                                  dimensions_panel[jpanel_surf].second;
            UVLM::Types::Block block_u_induced_x = u_induced_col_x.block(offset_col[icol_surf], offset_panel[jpanel_surf], k_surf_col_i, k_surf_panel_j);
            UVLM::Types::Block block_u_induced_y = u_induced_col_y.block(offset_col[icol_surf], offset_panel[jpanel_surf], k_surf_col_i, k_surf_panel_j);
            UVLM::Types::Block block_u_induced_z = u_induced_col_z.block(offset_col[icol_surf], offset_panel[jpanel_surf], k_surf_col_i, k_surf_panel_j);
            UVLM::Types::Block block_aic = aic_sources.block(offset_col[icol_surf], offset_panel[jpanel_surf], k_surf_col_i, k_surf_panel_j);

            if (options.Steady)
            {
                UVLM::UnitSourceDensity::get_influence_coefficient(zeta[jpanel_surf],
                                                                zeta_col[icol_surf],
																block_u_induced_x,
																block_u_induced_y,
																block_u_induced_z,
                                                                block_aic,
                                                                longitudinals_panel[jpanel_surf],
                                                                perpendiculars_panel[jpanel_surf],
                                                                normals_panel[jpanel_surf],
                                                                longitudinals_col[icol_surf],
                                                                perpendiculars_col[icol_surf],
                                                                normals_col[icol_surf]
                                                                );
            }
        }
    }
}

/*-----------------------------------------------------------------------------

-----------------------------------------------------------------------------*/
template <typename t_zeta_col,
          typename t_zeta_star,
          typename t_uext_col,
          typename t_gamma_star,
          typename t_normal>
void UVLM::Matrix::RHS
(
    const t_zeta_col& zeta_col,
    const t_zeta_star& zeta_star,
    const t_uext_col& uinc_col,
    const t_gamma_star& gamma_star,
    const t_normal& normal,
    const UVLM::Types::VMopts& options,
    UVLM::Types::VectorX& rhs,
    const uint& Ktotal
)
{
    const uint n_surf = options.NumSurfaces;

    rhs.setZero(Ktotal);

    // filling up RHS
    int ii = -1;
    int istart = 0;
    for (uint i_surf=0; i_surf<n_surf; ++i_surf)
    {
        uint M = uinc_col[i_surf][0].rows();
        uint N = uinc_col[i_surf][0].cols();

        if (!options.Steady)
        {
            #pragma omp parallel for collapse(2)
            for (uint i=0; i<M; ++i)
            {
                for (uint j=0; j<N; ++j)
                {
                    UVLM::Types::Vector3 v_ind;
                    UVLM::Types::Vector3 collocation_coords;
                    UVLM::Types::Vector3 u_col;

                    u_col << uinc_col[i_surf][0](i,j),
                             uinc_col[i_surf][1](i,j),
                             uinc_col[i_surf][2](i,j);
                    // we have to add the wake effect on the induced velocity.
                    collocation_coords << zeta_col[i_surf][0](i,j),
                                          zeta_col[i_surf][1](i,j),
                                          zeta_col[i_surf][2](i,j);

                    v_ind.setZero();
                    for (uint ii_surf=0; ii_surf<n_surf; ++ii_surf)
                    {
                        v_ind += UVLM::BiotSavart::whole_surface(zeta_star[ii_surf],
                                                                      gamma_star[ii_surf],
                                                                      collocation_coords,
                                                                      0,
                                                                      0,
                                                                      -1,
                                                                      -1,
                                                                      options.ImageMethod,
                                                                      options.vortex_radius);
                    }
                    u_col += v_ind;

                    // dot product of uinc and panel normal
                    uint counter = istart + j + i*N;
                    rhs(counter) =
                    -(
                        u_col(0)*normal[i_surf][0](i,j) +
                        u_col(1)*normal[i_surf][1](i,j) +
                        u_col(2)*normal[i_surf][2](i,j)
                    );
                }
            }
            istart += M*N;
        } else {
            for (uint i=0; i<M; ++i)
            {
                for (uint j=0; j<N; ++j)
                {
                    rhs(++ii) =
                    -(
                        uinc_col[i_surf][0](i,j)*normal[i_surf][0](i,j) +
                        uinc_col[i_surf][1](i,j)*normal[i_surf][1](i,j) +
                        uinc_col[i_surf][2](i,j)*normal[i_surf][2](i,j)
                    );
                }
            }
        }
    }
}

/*-----------------------------------------------------------------------------

-----------------------------------------------------------------------------*/
template <typename t_uext_col,
          typename t_normal>
void UVLM::Matrix:: RHS_nonlifting_body
(
    const t_uext_col& uinc_col,
    const t_normal& normal,
    UVLM::Types::VectorX& rhs,
    const uint& Ktotal,
    const uint n_surf
)
{
    rhs.setZero(Ktotal);

    // filling up RHS
    int ii = -1;
    int istart = 0;
    for (uint i_surf=0; i_surf<n_surf; ++i_surf)
    {
        uint M = uinc_col[i_surf][0].rows();
        uint N = uinc_col[i_surf][0].cols();
        for (uint i=0; i<M; ++i)
        {
            for (uint j=0; j<N; ++j)
            {
                rhs(++ii) =
                -(
                    uinc_col[i_surf][0](i,j)*normal[i_surf][0](i,j) +
                    uinc_col[i_surf][1](i,j)*normal[i_surf][1](i,j) +
                    uinc_col[i_surf][2](i,j)*normal[i_surf][2](i,j)
                );
            }
        }
    }
}


/*-----------------------------------------------------------------------------

-----------------------------------------------------------------------------*/

void UVLM::Matrix::generate_assembly_offset
(
    const UVLM::Types::VecDimensions& dimensions,
    const UVLM::Types::VecDimensions& dimensions_star,
    UVLM::Types::MatrixX& offset,
    const bool steady
)
{
    std::cerr << "Not sure this is correct, dont use (or debug)!!!" << std::endl;
    uint n_surf = dimensions.size();
    offset.setZero(n_surf, 2);

    uint counter = 0;
    for (uint i_surf=0; i_surf<n_surf; ++i_surf)
    {
        offset(i_surf, 0) = counter;
        counter += dimensions[i_surf].first*
                   dimensions[i_surf].second;

        offset(i_surf, 1) = counter;
        if (!steady)
        {
            counter += dimensions_star[i_surf].first*
                       dimensions_star[i_surf].second;
        }
    }
}

template <typename t_gamma,
          typename t_zeta_col>
void UVLM::Matrix::reconstruct_gamma
(
    const UVLM::Types::VectorX& gamma_flat,
    t_gamma& gamma,
    const t_zeta_col& zeta_col
)
{
    const uint n_surf = gamma.size();
    UVLM::Types::VecDimensions dimensions;
    UVLM::Types::generate_dimensions(zeta_col, dimensions);

    uint i_flat = 0;
    for (uint i_surf=0; i_surf<n_surf; ++i_surf)
    {
        for (uint i=0; i<dimensions[i_surf].first; ++i)
        {
            for (uint j=0; j<dimensions[i_surf].second; ++j)
            {
                gamma[i_surf](i, j) = gamma_flat(i_flat++);
            }
        }
    }

}
template <typename t_vec_mat,
		  typename t_zeta_col>
void UVLM::Matrix::reconstruct_MatrixX
(
    const UVLM::Types::MatrixX& mat_flat,
    t_vec_mat& vec_mat,
    const t_zeta_col& zeta_col
)
{
    const uint n_surf = zeta_col.size();
    UVLM::Types::VecDimensions dimensions;
    UVLM::Types::generate_dimensions(zeta_col, dimensions);

    uint i_flat = 0;
    for (uint i_surf=0; i_surf<n_surf; ++i_surf)
    {
        for (uint i=0; i<dimensions[i_surf].first; ++i)
        {
            for (uint j=0; j<dimensions[i_surf].second; ++j)
            {
				for (uint i_dim=0; i_dim<UVLM::Constants::NDIM; i_dim++)
				{
					vec_mat[i_surf][i_dim](i, j) = mat_flat(i_dim,i_flat);
				}
				i_flat++;
            }
        }
    }

}
template <typename t_gamma,
          typename t_zeta_col>
void UVLM::Matrix::deconstruct_gamma
(
    const t_gamma& gamma,
    UVLM::Types::VectorX& gamma_flat,
    const t_zeta_col& zeta_col
)
{
    const uint n_surf = gamma.size();
    UVLM::Types::VecDimensions dimensions;
    UVLM::Types::generate_dimensions(zeta_col, dimensions);

    uint n_total = 0;
    for (uint i_surf=0; i_surf<n_surf; ++i_surf)
    {
        n_total += dimensions[i_surf].first*dimensions[i_surf].second;
    }
    gamma_flat.resize(n_total);

    uint i_flat = 0;
    for (uint i_surf=0; i_surf<n_surf; ++i_surf)
    {
        for (uint i=0; i<dimensions[i_surf].first; ++i)
        {
            for (uint j=0; j<dimensions[i_surf].second; ++j)
            {
                gamma_flat(i_flat++) = gamma[i_surf](i, j);
            }
        }
    }

}

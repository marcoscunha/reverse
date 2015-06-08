#include <Processor/Processor.h>
#include <DnaTools/DnaTools.h>

#include <Private/Driver.h>
#include <Private/Dnp.h>

#ifdef DNP_RDMA_DEBUG
#define DEBUG
#endif

#define DBG_HDR "dnp_rdma:topo"
#include <Private/debug.h>

/*
 *       Static variables
 */

/** declaration  of the cartesian topology. */
static dnp_cart_topology_t cart_topo;

/** declaration of the  variable to store local coordinates */
static dnp_cart_coords_t  local_coords;

static void dnp_mesh_init(void);


int32_t
dnp_topo_init(void)
{

   uint32_t nnodes = 1;

   cart_topo . ndims = DNP_TOPOLOGY_INFO . mesh_ndims;

   for (int i=0; i <  cart_topo . ndims; i++)
   {
     nnodes = nnodes *  DNP_TOPOLOGY_INFO . mesh_dims[i];
     cart_topo . dim[i] = DNP_TOPOLOGY_INFO . mesh_dims[i];
     cart_topo . periods[i] = 1; // all dims are cyclic (torus)
   }
   
   cart_topo .nnodes = nnodes;

   // Compute and keep the local coordinates
   dnp_cart_coords(DNP_COMMON . local_rank, &local_coords);
   
   dnp_mesh_init();

   return 0;
}

/** dnp_cart_get_size
 *    @param dim : dimension for which the size is requested 
 *    @returns  size of the requested dimension
 *    Description: gets the size of a given dimension in current topology
 */
uint32_t dnp_cart_get_size(uint32_t dim)
{
    return  (dim <  cart_topo.ndims) ? cart_topo.dim[dim] : 0;
}

/** dnp_cart_coords
 *    @param rank : rank
 *    @param out_coords : computed coordinates
 *    @returns  : 0 in case of success 
 *    Description: computes the coordinates  corresponding to the
 *    given rank in the given topology
 */
int32_t dnp_cart_coords(uint32_t rank, dnp_cart_coords_t *out_coords)
{
#if 0
    uint32_t nnodes = cart_topo . nnodes;
#endif
	int32_t i = 0;    
#if 0
    for (i = 0; i < cart_topo . ndims; i++){ 
        nnodes = nnodes / cart_topo . dim[i];
        (*out_coords)[cart_topo . ndims - i - 1] = (uint32_t)(rank / nnodes); 
        rank = rank % nnodes;
    }
#else
    for(i = cart_topo.ndims - 1; i >= 0 ; i--){ 
        (*out_coords)[i] = (uint32_t)(rank % cart_topo.dim[i]); 
        rank = rank / cart_topo.dim[i];
    }
#endif

    return 0;
}

/**cart_get_local_coords
 *    @param coords : coordinates
 *    @param topo : topology
 *    @returns  : 0 in case of success 
 *    Description: computes the rank  corresponding to the
 *    coordinates in the given topology
 */
void dnp_cart_get_local_coords(dnp_cart_coords_t *coords)
{
    for (uint32_t i=0; i < cart_topo . ndims; i++)
    (*coords)[i] = local_coords[i];
}


#if 0


/** topo_rank
 *    @param in_coords: given coordinates
 *    @param rank : pointer to the computed rank
 *    @returns  : 0 in case of success 
 *    Description: computes the rank  corresponding to the
 *    coordinates in the given topology
 */
int32_t dnp_cart_rank(cart_coords_t *in_coords, uint32_t *rank)
{
    *rank = 0;

    for (int i=0; i <  cart_topo . ndims; i++)
    {
        *rank = *rank + (cart_topo . dim[i]) * ((*in_coords)[i]); 
    }

    return 0;
}

#endif 


/** dnp_cart_coordscmp
 *    @param coords1, coords2 : coordinates
 *    @returns  : 1 if equal, 0 else 
 *    Description: compares 2 coordinates in the given topology
 */
uint8_t
dnp_cart_coordscmp(dnp_cart_coords_t *coords1, dnp_cart_coords_t *coords2)
{
        
    for (int i=0; i < cart_topo . ndims; i++)
    {
        if ((*coords1)[i] != (*coords2)[i])
        {
            return 0; 
        }
    }

    return 1;
}

/** dnp_cart_setdim
 *    @param val : value 
 *    @param dim : dimension to set
 *    @param coords : coordinates
 *    Description: sets the dimension to value in the coordinates
 */
void
dnp_cart_set_dim(uint32_t val, uint32_t dim , dnp_cart_coords_t *coords)
{
    if  (dim <  cart_topo . ndims)
        (*coords)[dim] = val;
}

/** dnp_cart_getdim
 *    @param dim : dimension to set
 *    @param coords : coordinates
 *    @returns  value of the requested dimension
 *    Description: sets the dimension to value in the coordinates
 */
uint32_t
dnp_cart_get_dim(uint32_t dim, dnp_cart_coords_t *coords)
{
	 return  (dim <  cart_topo . ndims) ?  (*coords)[dim] : 0;
}



/*
 *    dnp_mesh_init
 *    Configures the lattice size register
 */
void
dnp_mesh_init(void)
{

	 uint32_t reg_val = 0;
	 dnp_cart_coords_t local;

	 dnp_cart_get_local_coords(&local);

	 DMSG("My position is : (%d, %d, %d) [%d,%d,%d]\n",
		  dnp_cart_get_dim(0, &local), dnp_cart_get_dim(1, &local), dnp_cart_get_dim(2, &local),
		  dnp_cart_get_size(0), dnp_cart_get_size(1), dnp_cart_get_size(2));

	 reg_val =
		  ((dnp_cart_get_size(0) << DNP_LATTICE_SIZE_X_O) & DNP_LATTICE_SIZE_X_M ) |
		  ((dnp_cart_get_size(1) << DNP_LATTICE_SIZE_Y_O) & DNP_LATTICE_SIZE_Y_M ) |
		  ((dnp_cart_get_size(2) << DNP_LATTICE_SIZE_Z_O) & DNP_LATTICE_SIZE_Z_M ) |
		  ((dnp_cart_get_dim(0, &local) << DNP_COORDS_X_O) & DNP_COORDS_X_M ) |
		  ((dnp_cart_get_dim(1, &local) << DNP_COORDS_Y_O) & DNP_COORDS_Y_M ) |
		  ((dnp_cart_get_dim(2, &local) << DNP_COORDS_Z_O) & DNP_COORDS_Z_M );

	 // write it to the DNP register
	 dnp_reg_write(GLBCFG_IDX, reg_val);
}


#if 0

/*
 *  dnp_mesh_set_local
 *  
 */
void dnp_mesh_set_local()
{
    cart_coords_t local_coords;
    DNP_REG coordsme, mychip3dcoord;

    cart_get_local_coords(&local_coords);

    // create register value      
    coordsme   = ((cart_get_dim(0, &local_coords) << X_OFFS)  & X_MASK) |
                 ((cart_get_dim(1, &local_coords) << Y_OFFS)  & Y_MASK) |
                 ((cart_get_dim(2, &local_coords) << Z_OFFS)  & Z_MASK);

   // to change for SHAPOTTO
    mychip3dcoord = coordsme;

    // write it to the DNP register
    dnp_reg_write((DNP_REG *)DNP_REG_3DCOORD_ME, coordsme);
    dnp_reg_write((DNP_REG *)DNP_REG_MYCHIP_3DCOORD, mychip3dcoord);
}


#endif /* 0 */

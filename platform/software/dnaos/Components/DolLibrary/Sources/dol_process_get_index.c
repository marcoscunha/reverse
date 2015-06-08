#include <Private/DolLibrary.h>

int32_t dol_process_get_index (DOLProcess * p, int32_t dim)
{
  dol_process_info_t * info = p -> wptr;

  switch (dim)
  {
    case 1 : return info -> index . x;
    case 2 : return info -> index . y;
    case 3 : return info -> index . z;
    default: return 0;
  }

	return 0;   
}

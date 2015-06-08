
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stddef.h>
#include <getopt.h>
#include <assert.h>

#include "events/hwe_tools.h"

#define BUFSIZE 1000000

void print_usage(FILE *f, char *arg0)
{
   fprintf(f, 
   "usage: %s [-h] [-fb] [-c id] [-n nb] <tracefile>\n"
   "Print the events of the tracefile.\n"
   "Options:\n"
   " -h: print this message\n"
   " -f: only print the firsts INFO events\n"
   " -c id: only print the events of component 'id'\n"
   " -C id: only print the events which have the 'id'\n"
   " -n nb: stop after printing 'nb' events\n"
   " -b: switch to binary mode\n"
	"Options for ascii mode:\n"
   " -s: don't print names of components\n"
   " -i/I: print 1liner for/don't print INST events\n"
   " -m/M: print 1liner for/don't print MEM events\n"
   " -a/A: print 1liner for/don't print ACK events\n"
   " -d/D: print 1liner for/don't print ID events\n"
   " -e/E: print 1liner for/don't print EXCEP events\n"
   , arg0);
}

static hwe_id_t ref2id(hwe_ref_t ref) {
	return *((hwe_id_t *) ref);
}

/********
 * MAIN *
 ********/
int main (int argc, char **argv)
{
   unsigned ncomp = 0;
   int mode[100] = { 0 };
   
   //option parsing
   int opt;
   bool *filter_id = NULL;
   bool only_info = false;
   bool filter_id_en = false;
	bool display_names = true;
	bool binary_mode = false;
	int  stop_after  = -1;
	unsigned long long nbev = 0;
	bool only_1id = false;
	unsigned long look_1id = 0;
   while ((opt = getopt(argc, argv, "hfbn:c:C:siImMaAdDeE")) != -1) {
      switch (opt) {
#define CASE(hwetype, short_opt, no_opt) \
         case short_opt : \
            mode[ hwetype ] = 1; \
            break; \
         case no_opt : \
            mode[ hwetype ] = 2; \
            break;

         CASE(HWE_INST32, 'i', 'I');
         CASE(HWE_MEM32, 'm', 'M');
         CASE(HWE_ID, 'd', 'D');
         CASE(HWE_EXCEP32, 'e', 'E');
         CASE(HWE_MEMACK, 'a', 'A');

#undef CASE
         case 'f':
            only_info = true;
            break;
         case 'b':
            binary_mode = true;
            break;
         case 'n':
            stop_after = atoi(optarg);
            break;
         case 's':
				display_names = false;
				break;
         case 'C':
				only_1id = true;
            look_1id = strtoul(optarg, NULL, 0);
				if (stop_after < 0)
					stop_after = 1;
            break;
         case 'c':
            {
               filter_id_en = true;
               unsigned id = atoi(optarg);
               if (id >= ncomp) {
                  unsigned nold = ncomp;
                  ncomp = id + 10;
                  filter_id = realloc(filter_id, sizeof(bool) * ncomp);
                  for (unsigned i = nold; i < ncomp; i += 1)
                     filter_id[i] = false;
               }
               filter_id[id] = true;
            }
            break;
         case 'h':
            print_usage(stdout, argv[0]);
            exit(EXIT_SUCCESS);
            break;
         default://'?'
            print_usage(stderr, argv[0]);
            exit(EXIT_FAILURE);
      }
   }

   /*
    *  initializing
    */
   // file and buffer
   int fd = open(argv[optind], O_RDONLY);
   void *buf = malloc(BUFSIZE);
   void *bcur = buf;
   size_t bsize = 0;

   // current id array
   hwe_id_ind_t *previd = calloc(ncomp, sizeof(hwe_id_ind_t));
   hwe_id_ind_t *writeid = calloc(ncomp, sizeof(hwe_id_ind_t));
   char **names = calloc(ncomp, sizeof(char*));

 	if (binary_mode)
	  fprintf(stderr, "Binary mode enabled\n");	

   /*
    * fetch events one by one
    */
	bool eof = false, sof = true;
#define MAXCONT 10
	unsigned curcont = 0;//index in following arrays
	hwe_cont event[MAXCONT];//for event containers
	struct {
		hwe_id_t ids[HWE_REF_MAX];	
	} event_refs[MAXCONT];// for event references 
   
	do {//loop on every "real" containers (ie: not a HWE_ID)
		assert(curcont < MAXCONT);

		do {//handle HWE_ID special containers
			/*
			 *  read one container from the file
			 */
			size_t used;
			if ((used = hwe_read(&event[curcont], &event_refs[curcont].ids[0], bcur, bsize, previd, ncomp)) == 0) {
				/*
				 * buffer does not contain enough data
				 * We need to refill it
				 */

				//fprintf(stdout, "Buffer re-filling (still %u bytes)...\n", (unsigned) bsize);
				if (bsize > (BUFSIZE / 2)) {
					fprintf(stderr, "Error: re-filling a half-full buffer (the buffer is undersized).\n");
					exit(1);
				}
				//buffer should be almost empty so memcpy and not memove
				bcur = memcpy(buf, bcur, bsize);
				ssize_t r = read(fd, buf + bsize, BUFSIZE - bsize);
				if (r <= 0) {
					if (r < 0) {
						fprintf(stderr, "Error when trying to read from trace file\n");
						exit(1);
					}
					//end of file
					eof = true;
					break;
				}
				bsize += r;
				if ((used = hwe_read(&event[curcont], &event_refs[curcont].ids[0], bcur, bsize, previd, ncomp)) == 0) {
					fprintf(stderr, "Error: couldn't read event (the buffer is undersized).\n");
					exit(1);
				}
			}
			bcur += used;
			bsize -= used;

			/*
			 * updating previous id array (it's the only thing to do for an HWE_ID)
			 * eventually widen comp/previd arrays
			 */
			hwe_id_ind_t id = event[curcont].common.id.devid;
			if (ncomp <= id) {
				unsigned nold = ncomp;
				ncomp = id + 10;
				
				previd = realloc(previd, sizeof(hwe_id_ind_t) * ncomp);
				writeid = realloc(writeid, sizeof(hwe_id_ind_t) * ncomp);
				names = realloc(names, sizeof(char *) * ncomp);
				if (filter_id_en)
					filter_id = realloc(filter_id, sizeof(bool) * ncomp);
				
				for (unsigned i = nold; i < ncomp; i += 1) {
					previd[i] = 0;
					writeid[i] = 0;
					names[i] = NULL;
					if (filter_id_en)
						filter_id[i] = false;
				}
			}
			previd[id] = event[curcont].common.id.index;
		} while (event[curcont].common.head.type == HWE_ID);
		//at this point we have a container or eof

		/*
		 * is this the end of the stored event
		 */
		if (!sof && (
					event[curcont].common.id.devid != event[0].common.id.devid || 
					event[curcont].common.id.index != event[0].common.id.index || 
					eof) ) {

			unsigned id = event[0].common.id.devid;
			// handle the stored event
			if (event[0].common.head.type == HWE_INFO) {
				// register the name
				if (names[id] == NULL) {
					names[id] = malloc(10U + event[0].info.body.nsize);
					sprintf(names[id], "<%s> ",
							&event[0].info.name[0]);
				}
			} else if (only_info) {
				//we stop here
				break;
			}

			// print it ?
			if (!filter_id_en || filter_id[id]) {
				if (!only_1id || look_1id <= event[0].common.id.index) {
					if (!binary_mode) {
						switch (mode[event[0].common.head.type]) {
							case 1:
								{
									char desc[60];
									hwe_desc(&event[0], ref2id, desc, 60);
									fprintf(stdout, "%sHWE %s\n", (display_names && names[id])?names[id]:"", desc);
									nbev++;
									if (stop_after > 0)
										stop_after--;
								}
								break;
							case 2:
								break;

							default:
								hwe_print(stdout, &event[0], ref2id, display_names?names[id]:NULL, NULL);
								nbev++;
								if (stop_after > 0)
									stop_after--;
								break;
						}
					} else {
						nbev++;
						char writebuffer[sizeof(hwe_cont)];
						hwe_cont *hwe = &event[0];
							
						// if necessary insert a HWE_ID
						if (!hwe_head_rid_compute(&hwe->common, writeid[id])) {
								hwe_id_write(&hwe->common, writebuffer);
								fwrite(writebuffer, 1, HWE_ID_SIZEOF, stdout);
						}
						writeid[id] = hwe->common.id.index;
					
						do {
							hwe_id_t ids[HWE_REF_MAX];
							for (unsigned i = 0; i < hwe_getnref(&hwe->common); i += 1) {
								ids[i] = ref2id(hwe_getref(&hwe->common, i));
							}
							size_t size = hwe_sizeof(hwe);
							hwe_write(hwe, &ids[0], writebuffer);
							fwrite(writebuffer, 1, size, stdout);
							hwe = (hwe_cont *) hwe->common.refnext;
						} while (hwe != NULL);
						if (stop_after > 0)
							stop_after--;
					}
				}
			}

			//prepare things for the new event
			memcpy(&event[0], &event[curcont], sizeof(hwe_cont));
			for (unsigned i = 0; i < event[0].common.head.nrefs; i++)
				event_refs[0].ids[i] = event_refs[curcont].ids[i];
			curcont = 0;
		}

		//ensure references of the container are ok (for printing)
		//we put a pointer to an hwe_id_t into it
		for (unsigned i = 0; i < event[curcont].common.head.nrefs; i++) {
			event[curcont].common.refs[i] = (hwe_ref_t) &event_refs[curcont].ids[i];
		}
		if (curcont != 0) {
			event[curcont-1].common.refnext = &event[curcont].common;
			event[curcont].common.reflast = NULL;
		}
		event[curcont].common.refnext = NULL;
		event[0].common.reflast = &event[curcont].common;

		curcont += 1;

		sof = false;
	} while (!eof && stop_after != 0);

   /*
    * closing
    */
	fprintf(stderr, "Read %llu events\n", nbev);
   free(previd);
   free(writeid);
   for (unsigned i = 0; i < ncomp; i += 1)
      if (names[i])
         free(names[i]);
   free(names);
   free(buf);
   if (filter_id_en)
      free(filter_id);
   close(fd);

}


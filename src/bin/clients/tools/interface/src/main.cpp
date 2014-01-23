#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include "stinger_core/stinger.h"
#include "stinger_core/xmalloc.h"
#include "stinger_net/mon_handling.h"
#include "stinger_net/stinger_mon.h"
#include "stinger_utils/timer.h"


using namespace gt::stinger;


int main (int argc, char *argv[])
{

    
    /* Connect to STINGER as a monitor */
    if (-1 == mon_connect(10103, "localhost", "alg_to_mongo")) {
        LOG_E_A ("Connect to STINGER on %s:%d failed", "localhost", 10103);
        return -1;
    }
    
    StingerMon & mon = StingerMon::get_mon();

    sleep(4);
    while (1)
    {
        mon.wait_for_sync();

        
        /* Get the STINGER pointer -- critical section */
        mon.get_alg_read_lock();
        
        stinger_t * S = mon.get_stinger();
        if (!S) {
            LOG_E ("bad stinger pointer");
            return -1;
        }
        

        /*
         if (!alg_state) {
         mon.release_alg_read_lock();
         LOG_E_A ("Algorithm %s does not exist", argv[algorithm_name_index]);
         exit(-1);
         }
         */
        
        //get graph information
        int64_t nv = stinger_mapping_nv(S);
        printf("%ld vertices\n",nv);
        
        for (int64_t i=0;i<=stinger_max_active_vertex(S);i++){
            printf("%ld has outdegree %ld\n", i, stinger_outdegree(S,i));
            STINGER_FORALL_EDGES_OF_VTX_BEGIN(S,i){
                printf("%ld -> %ld (weight %ld)\n",i,STINGER_EDGE_DEST,STINGER_EDGE_WEIGHT);
            }STINGER_FORALL_EDGES_OF_VTX_END();
        }
        
        
        //get the number of algorithms
        size_t num_algs=mon.get_num_algs();
        printf("There are %zu algs running\n",num_algs);
        
        //get pagerank algorithm and data
        StingerAlgState * alg_state = mon.get_alg("pagerank");
        if(alg_state){
            //get algorithm name
            const char * alg_name = alg_state->name.c_str();
            printf("Algorithm %s\n",alg_name);
            //get algorithm data description
            char * tmp = (char *) xmalloc ((alg_state->data_description.length()+10) * sizeof(char));
            strcpy(tmp, alg_state->data_description.c_str());
            printf("%s\n",tmp);
            
            //get a copy of the alg data pointer
            double *data=(double *)alg_state->data;
            //get data size per vertex
            int64_t datapervertex=alg_state->data_per_vertex;
            
            
            for(int64_t v=0;v<nv;v++){
                printf("%ld = %f\n",v,data[v]);
            }
        }
        alg_state = mon.get_alg("static_components");
        if(alg_state){
            //get algorithm name
            const char * alg_name = alg_state->name.c_str();
            printf("Algorithm %s\n",alg_name);
            //get algorithm data description
            char * tmp = (char *) xmalloc ((alg_state->data_description.length()+10) * sizeof(char));
            strcpy(tmp, alg_state->data_description.c_str());
            printf("%s\n",tmp);
            
            //get a copy of the alg data pointer
            int64_t *data=(int64_t *)alg_state->data;
            //get data size per vertex
            int64_t datapervertex=alg_state->data_per_vertex;
            
            
            for(int64_t v=0;v<nv;v++){
                printf("%ld = %ld\n",v,data[v]);
            }
        }
        
        
        /*can cycle through algorithms as below, but currently poses problems accessing data*/
        /*
        for (size_t i=0;i<num_algs;i++){
            
            //StingerAlgState * alg_state = mon.get_alg(i);
            StingerAlgState * alg_state = mon.get_alg("pagerank");
            const char * alg_name = alg_state->name.c_str();
            printf("Alg %s\n",alg_name);
            char * tmp = (char *) xmalloc ((alg_state->data_description.length()+10) * sizeof(char));
            strcpy(tmp, alg_state->data_description.c_str());
            printf("%s\n",tmp)
            
            uint64_t *data=(uint64_t *)alg_state->data;
            int64_t datapervertex=alg_state->data_per_vertex;
            
            printf("%ld data per vertex\n",datapervertex);
            for(int64_t v=0;v<nv;v++){
                printf("%ld = %f\n",v,*((double*)data+v));
            }
        }
*/
        
        
        /* Release the lock */
        mon.release_alg_read_lock();

    } /* end infinite loop */

    return 0;
}



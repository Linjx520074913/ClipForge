import { MapMetaData, MapRouteGraph } from "@core/datasource/IDataService";
import { computed, ref } from "vue";

// export const useMapStore = defineStore('map', {
//     state: () => ({
//         mapList: [] as MapMetaData[],
//         routeGraph: {} as MapRouteGraph,
//         activedIndex: 0
//     }),
//     getters:{
//         curMap: (state): MapMetaData | undefined => {
//             return state.mapList[state.activedIndex];
//           }
//     },
//     actions: {
//         setMapList(list: MapMetaData[]){
//             this.mapList = list
//             console.error('@@@@@@', this.activedIndex)
//         },
//         setMapRouteGraph(value: MapRouteGraph){
            
//             this.routeGraph = value
//             console.error('设置地图路径图', this.routeGraph, value)
//         }
//     }
// })

const MapStoreRef = ref({
    data: {
        mapList: [] as MapMetaData[],
        routeGraph: {} as MapRouteGraph,
        activedIndex: 0
    },
    methods: {
        setMapList(list: MapMetaData[]) {
            MapStore.data.mapList = list
        },
        setMapRouteGraph(value: MapRouteGraph) {
            MapStore.data.routeGraph = value
            console.error('设置地图路径图', MapStore.data, value)
        }
    },
})

export const MapStore = MapStoreRef.value
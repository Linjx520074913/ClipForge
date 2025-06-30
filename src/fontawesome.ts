import { library } from '@fortawesome/fontawesome-svg-core';
import { FontAwesomeIcon } from '@fortawesome/vue-fontawesome';
import { 
    faPlus, 
    faMinus, 
    faArrowsToDot, 
    faExchangeAlt, 
    faMapMarker, 
    faTrash, 
    faExclamationTriangle, 
    faLayerGroup, 
    faInfoCircle, 
    faLocation, 
    faSlidersH, 
    faCheck, 
    faMousePointer,
    faSitemap,
    faFastBackward,
    faFastForward,
    faPlayCircle,
    faUndo
} from '@fortawesome/free-solid-svg-icons'; // 你用到的图标

library.add(faTrash);
library.add(faExclamationTriangle);
library.add(faLayerGroup)
library.add(faInfoCircle)
library.add(faLocation)
library.add(faSlidersH)
library.add(faCheck)
library.add(faMousePointer)
library.add(faMapMarker)
library.add(faExchangeAlt)
library.add(faPlus)
library.add(faMinus)
library.add(faArrowsToDot)
library.add(faSitemap)
library.add(faFastBackward)
library.add(faFastForward)
library.add(faPlayCircle)
library.add(faUndo)

export { library, FontAwesomeIcon }
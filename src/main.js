// virual human webclient.
// fcx@pingxingyun.com
//

import { createApp } from 'vue'
import App from './App.vue'
import './asstes/styles/main.css'
import { library } from "@fortawesome/fontawesome-svg-core";
import { faPhone, faMicrophoneAlt, faMicrophone, faTrash, faUserCircle, faUserAstronaut } from "@fortawesome/free-solid-svg-icons";
import { FontAwesomeIcon } from "@fortawesome/vue-fontawesome";

library.add(faPhone, faMicrophoneAlt, faMicrophone, faTrash, faUserCircle, faUserAstronaut);

createApp(App)
    .component("font-awesome-icon", FontAwesomeIcon)
    .mount('#app')

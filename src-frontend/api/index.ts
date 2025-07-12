import axios from 'axios';

const AXIOS = axios.create({
    baseURL: 'http://127.0.0.1:3000'          // 测试环境服务器
    // baseURL: 'http://43.138.212.12' // 在线环境服务器
})

AXIOS.interceptors.request.use(
    
    function(config: any){

        // const user = User.getCurUser();
        // if(user != undefined){
        //     config.headers.Authorization = 'Bearer ' + user.token
        // }

        return config

    },
    function(error: any){

        return Promise.reject(error)

    }

)

export { AXIOS }

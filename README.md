#关于funginx  
*funginx是一个比较闲得慌的创作版本，主要特点在于以下：*  
  **<1>删除了所有的debug、所有的log_debug函数、适配win32的预编译选项**  
  **<2>增加了俩比较闲得慌的模块儿，访问指定路径/zero或/four会显示诗句**  
  **<3>有些core模块里边增加了重试15遍的for循环，防止仅仅申请或删除一次内存空间失败，就返回错误NGX_ERROR**  
  **<4>所有special_response的错误显示全换成了文言文而且页面字体改变成了楷体，utf-8**  
  **<5>添加进去一个logger包，是的，我把官方的debug的log功能全删了，然后添上了自己的，叫fun_log**  
#特别注意  
*编译的时候我自己仅仅是切换到根目录下就直接执行了“./auto/configure”然后就开始“make && make install”，但可能由于更改问题，生成的objs/ngx_module.c里边总会多出来'^M'这么个狗东西，需要手动删掉之后，才能“make && make install”，十分不好意思不知道为什么会这样*


#about funginx  
*funginx is a relatively idle creative version, the main features are as follows:*  
  **<1>Deleted all debug, all log_debug functions, precompiled options for win32**  
  **<2> Added two more idle modules, visiting the specified path /zero or /four will display verses**  
  **<3>In some core modules, a for loop that retries 15 times is added to prevent the error NGX_ERROR from being returned if the application or deletion of memory space fails only once**  
  **<4>The error display of all special_response has been changed to classical Chinese and the font of the page has been changed to italics, utf-8**  
  **<5>Add a logger package, yes, I deleted all the official debug log functions, and then added my own, called fun_log**  
#pay attention  
*When compiling, I just switched to the root directory and directly executed "./auto/configure" and then started "make && make install", but it may be due to changes, the generated objs/ngx_module.c will always be There is such a thing as '^M', which needs to be deleted manually before "make && make install". I'm very sorry and don't know why it happened*


#關於funginx  
*funginx是一個比較閒得慌的創作版本，主要特點在於以下：*  
  **<1>刪除了所有的debug、所有的log_debug函數、適配win32的預編譯選項**  
  **<2>增加了倆比較閒得慌的模塊兒，訪問指定路徑/zero或/four會顯示詩句**  
  **<3>有些core模塊裡邊增加了重試15遍的for循環，防止僅僅申請或刪除一次內存空間失敗，就返回錯誤NGX_ERROR**  
  **<4>所有special_response的錯誤顯示全換成了文言文而且頁面字體改變成了楷體，utf-8**  
  **<5>添加進去一個logger包，是的，我把官方的debug的log功能全刪了，然後添上了自己的，叫fun_log**  
#特別注意  
*編譯的時候我自己僅僅是切換到根目錄下就直接執行了“./auto/configure”然後就開始“make && make install”，但可能由於更改問題，生成的objs/ngx_module.c裡邊總會多出來'^M'這麼個狗東西，需要手動刪掉之後，才能“make && make install”，十分不好意思不知道為什麼會這樣*


 ![image](https://user-images.githubusercontent.com/125577583/221465212-cae7b33d-6d2b-4ef3-9106-3282db6e44dc.png)
 ![image](https://user-images.githubusercontent.com/125577583/221465241-9ba6bc4a-7c80-4b40-a092-8f896bb5dcc2.png)
 ![image](https://user-images.githubusercontent.com/125577583/221465274-8c5bb486-c205-4a03-9102-bed0bcb0f1b6.png)

#include <iostream>
#include <fstream>
#include <filesystem>
#include <sstream>
#include <string>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <chrono>
#include <thread>
#include <vector>
#include <algorithm>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <pthread.h>
#include <curl/curl.h>
#include <sys/stat.h>
#include <fstream>
#include <miniz.h>

using namespace std;
namespace fs = std::filesystem;
// 基础函数定义

//自动下载程序所需要的库

bool isLibraryInstalled(const std::string& libraryName) {
    std::string command = "dpkg -s " + libraryName + " 2>/dev/null";
    return (system(command.c_str()) == 0);
}

bool installLibrary(const std::string& libraryName) {
    std::string command = "sudo apt-get update && sudo apt-get install -y " + libraryName;
    return (system(command.c_str()) == 0);
}

// 是否为初次启动

bool isFirstTimeStartup() {
    struct stat st;
    if (stat("BDS", &st) == 0 && S_ISDIR(st.st_mode)) {
        return false;
    }
    return true;
}

bool createRootDirectory() {
    if (mkdir("BDS", 0777) == 0) {
        std::cout << "Root directory 'BDS' created successfully." << std::endl;
        return true;
    } else {
        std::cerr << "Failed to create the root directory 'BDS'." << std::endl;
        return false;
    }
}

// 下载并解压压缩文件

// 回调函数，用于libcurl接收下载数据
size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* output) {
    size_t totalSize = size * nmemb;
    output->append(static_cast<char*>(contents), totalSize);
    return totalSize;
}

// 下载并解压BDS压缩文件
bool DownloadAndExtract(const std::string& url, const fs::path& targetDir) {
    // Step 1: 使用libcurl下载文件
    CURL* curl = curl_easy_init();
    if (!curl) {
        std::cerr << "无法初始化libcurl。" << std::endl;
        return false;
    }

    std::string downloadedData;
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &downloadedData);

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        std::cerr << "无法从 " << url << " 下载文件：" << curl_easy_strerror(res) << std::endl;
        curl_easy_cleanup(curl);
        return false;
    }

    curl_easy_cleanup(curl);

    // Step 2: 将下载的数据保存到目标目录中
    fs::path filePath = targetDir / "bedrock-server-1.20.1.02.zip";
    std::ofstream ofs(filePath, std::ios::binary);
    if (!ofs) {
        std::cerr << "无法打开文件 " << filePath << " 进行写入。" << std::endl;
        return false;
    }

    ofs << downloadedData;
    ofs.close();

    // Step 3: 使用miniz解压缩zip文件
    mz_zip_archive zip;
    memset(&zip, 0, sizeof(zip));
    if (!mz_zip_reader_init_file(&zip, filePath.string().c_str(), 0)) {
        std::cerr << "无法初始化miniz zip reader。" << std::endl;
        return false;
    }

    mz_zip_reader_extract_all(&zip, targetDir.string().c_str(), 0);
    mz_zip_reader_end(&zip);

    std::cout << "BDS压缩文件下载并解压成功。" << std::endl;

    return true;
}

// 判断BDS目录下是否存在bedrock_server.exe文件
bool CheckBDSInstallation(const std::string& bdsDir);

// 匿名管道功能相关定义和实现
// TCP功能相关定义和实现
// ...

void handleClient(int sockfd) {
    char buffer[256];
    ssize_t bytesRead = recv(sockfd, buffer, sizeof(buffer), 0);
    if (bytesRead == -1) {
        std::cerr << "无法接收数据。" << std::endl;
    } else if (bytesRead == 0) {
        // 客户端关闭了连接
        std::cout << "客户端关闭了连接。" << std::endl;
    } else {
        // 在这里处理接收到的消息
        std::string message(buffer, bytesRead);
        std::cout << "接收到来自客户端的消息：" << message << std::endl;

        // 可选：发送响应给客户端
        std::string response = "服务器收到消息：" + message;
        send(sockfd, response.c_str(), response.size(), 0);
    }

    close(sockfd); // 关闭与客户端的连接
}

//获取基本系统信息
std::string executeCommand(const std::string& command) {
    std::string result;
    char buffer[128];
    FILE* pipe = popen(command.c_str(), "r");
    if (!pipe) {
        return "ERROR";
    }
    while (!feof(pipe)) {
        if (fgets(buffer, 128, pipe) != nullptr) {
            result += buffer;
        }
    }
    pclose(pipe);
    return result;
}

// 基础文件操作功能
bool CreateFile(const std::string& filename);
bool ModifyFile(const std::string& filename, const std::string& content);
bool ReadFile(const std::string& filename, std::string& content);
bool DeleteFile(const std::string& filename);

// 服务端更新
// 删除文件
bool deleteFile(const fs::path& filePath) {
    try {
        return fs::remove(filePath);
    } catch (const fs::filesystem_error& ex) {
        std::cerr << "无法删除文件：" << ex.what() << std::endl;
        return false;
    }
}

// 下载回调函数，用于保存下载的数据
size_t writeCallback(void* contents, size_t size, size_t nmemb, std::string* output) {
    size_t totalSize = size * nmemb;
    output->append(static_cast<char*>(contents), totalSize);
    return totalSize;
}

// 下载文件
bool downloadFile(const std::string& url, const fs::path& downloadPath) {
    CURL* curl = curl_easy_init();
    if (!curl) {
        std::cerr << "无法初始化CURL。" << std::endl;
        return false;
    }

    std::string downloadedData;
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &downloadedData);

    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
        std::cerr << "下载文件失败：" << curl_easy_strerror(res) << std::endl;
        return false;
    }

    try {
        // 将下载的数据保存为文件
        std::ofstream ofs(downloadPath, std::ios::binary);
        if (!ofs.is_open()) {
            std::cerr << "无法保存下载的文件。" << std::endl;
            return false;
        }
        ofs.write(downloadedData.c_str(), downloadedData.size());
        ofs.close();
        return true;
    } catch (const std::exception& ex) {
        std::cerr << "保存下载的文件失败：" << ex.what() << std::endl;
        return false;
    }
}

// 解压缩文件
bool unzipFile(const fs::path& zipPath, const fs::path& unzipDir) {
    try {
        int res = mz_zip_extract(zipPath.string().c_str(), unzipDir.string().c_str(), nullptr, nullptr, 0);
        if (res == MZ_FALSE) {
            std::cerr << "解压文件失败。" << std::endl;
            return false;
        }
        return true;
    } catch (const std::exception& ex) {
        std::cerr << "解压文件失败：" << ex.what() << std::endl;
        return false;
    }
}

// 检测程序是否运行
bool isBdsRunning(const std::string& bdsFileName) {
    // 构造命令来检查进程是否在运行
    std::string command = "pgrep -x " + bdsFileName;

    // 执行命令，并读取输出
    FILE* fp = popen(command.c_str(), "r");
    if (!fp) {
        std::cerr << "无法执行命令。" << std::endl;
        return false;
    }

    char buffer[256];
    std::string result;
    while (fgets(buffer, sizeof(buffer), fp)) {
        result += buffer;
    }
    pclose(fp);

    // 如果输出结果非空，则表示BDS在运行
    return !result.empty();
}

//程序主体
int main() {
    //程序初始化
    std::string bdsFileName = "bedrockserver";
    std::string downloadUrl = "https://minecraft.azureedge.net/bin-linux/bedrock-server-1.20.1.02.zip";
    fs::path bdsDir = "BDS";
    fs::path downloadDir = bdsDir / "download";
    fs::path zipFilePath = downloadDir / "bedrock-server-1.20.1.02.zip";
    if (isFirstTimeStartup()) {
        if (createRootDirectory()) {
            // 在这里执行其他初次启动的操作

            std::cout << "此为第一运行ZeroClear BDS面板，接下来将进行面板初始化。" << std::endl;
            
            // 检查是否已安装OpenSSL库

            if (!isLibraryInstalled("libssl-dev")) {
                std::cout << "未安装OpenSSL库，尝试安装..." << std::endl;

            // 尝试安装库

            if (installLibrary("libssl-dev")) {
                std::cout << "OpenSSL库安装成功。" << std::endl;
            }

            else {
            std::cerr << "无法安装OpenSSL库，请手动安装后重试。" << std::endl;
            return 1;
            }
            }
            
            // 创建根目录

            std::string bdsDir = "BDS"; // BDS目录名
            if (mkdir(bdsDir.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) != 0) {
            std::cerr << "创建BDS目录失败" << std::endl;
            }

            // 配置tcp通信密钥(此处需加密：16进制/其他加密手段)
            fs::path rootDir = "BDS";
            fs::path configFile = rootDir / "config.cfg";

            if (!fs::exists(rootDir) || !fs::is_directory(rootDir)) {
                if (fs::create_directory(rootDir)) {
                    std::cout << "根目录 'BDS' 创建成功。" << std::endl;
                }
                else {
                    std::cerr << "无法创建根目录 'BDS'。" << std::endl;
                    return 1;
                }
            }

            std::string username, password;
            std::cout << "请输入用户名：";
            std::cin >> username;
            std::cout << "请输入密码：";
            std::cin >> password;
            std::string token = generateRandomToken();
            std::string encryptedToken = encryptToken(token);

            std::ofstream ofs(configFile);
            if (ofs.is_open()) {
                ofs << "用户名：" << username << std::endl;
                ofs << "密码：" << password << std::endl;
                ofs << encryptedToken;
                ofs.close();
                std::cout << "用户名、密码和令牌已加密并写入 'config.cfg' 文件。" << std::endl;
            } 
            else {
                std::cerr << "无法打开 'config.cfg' 文件进行写入。" << std::endl;
                return 1;
            }

            //自动下载最新版本BDS并存储到BDS目录

            // 下载并解压BDS压缩文件

            std::string url = "https://minecraft.azureedge.net/bin-win/bedrock-server-1.20.1.02.zip";
            fs::path bdsDir = rootDir / "bedrock-server";
            if (!DownloadAndExtract(url, bdsDir)) {
                std::cerr << "[BDS_Consolo][错误] BDS安装失败。" << std::endl;
                return 1;
            }

            else {
                std::cerr << "[BDS_Consolo][INFO]BDS安装成功" << std::endl;
                std::cerr << "即将进入BDS初次部署并运行阶段。" << std::endl;
            }
        } 
        else {
            // 处理创建目录失败的情况

            std::cout << "程序初始化失败,请重试。" << std::endl;

        }
    } 
    
    else {
        std::cout << "欢迎回来！Zeroclear BDS面板正在启动。" << std::endl;
        // 在这里执行其他非初次启动的操作
        
        // 配置TCP端口和密码等信息
        // 读取config文件里的用户名密码和加密令牌

        std::ifstream ifs(configFile);
        if (ifs.is_open()) {
        std::string token, username, password;
        std::getline(ifs, token); // 读取第一行为令牌
        std::getline(ifs, username); // 读取第二行为用户名
        std::getline(ifs, password); // 读取第三行为密码

        // 清除行末的换行符
        if (!token.empty() && token.back() == '\n') {
            token.pop_back();
        }
        if (!username.empty() && username.back() == '\n') {
            username.pop_back();
        }
        if (!password.empty() && password.back() == '\n') {
            password.pop_back();
        }
        //将数据更新到新的变量中
        string token1 = token;//更新tocken
        string username1 = username;//更新username
        string password1 = password;//更新password

        ifs.close();
        }
        else {
            std::cerr << "无法打开 'config.cfg' 文件进行读取。" << std::endl;
            return 1;
        }

        // 检查BDS是否安装成功
        if (!CheckBDSInstallation(bdsDir)) {
            std::cerr << "[BDS_Consolo][ERROR]BDS未安装，请删除所有文件后重启程序" << std::endl;
            return 1;
        }

        else{
            std::cout << "[BDS_Consolo][INFO]BDS安装成功，正在启动BDS服务端" << std::endl;
        }

        // 运行时程序
        //显示cpu和内存信息
        std::string cpuInfo = executeCommand("lscpu");
        std::string memoryInfo = executeCommand("free -h");
        std::string diskInfo = executeCommand("lsblk");
    
        std::cout << "CPU信息:\n" << cpuInfo << std::endl;
        std::cout << "内存信息:\n" << memoryInfo << std::endl;
        std::cout << "磁盘信息:\n" << diskInfo << std::endl;
    
        // 初始化匿名管道功能
        // ...
        // 创建一个Socket
         int sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd == -1) {
            std::cerr << "无法创建Socket。" << std::endl;
            return 1;
        }
        // 配置服务器地址和端口
        struct sockaddr_in serverAddr;
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(10086); // 设置监听的端口号10086
        serverAddr.sin_addr.s_addr = INADDR_ANY;

        // 处理客户端链接，创建新线程，并行处理
        // ...
        std::cout << "服务器已启动，等待客户端连接..." << std::endl;

        while (true) {
            struct sockaddr_in clientAddr;
            socklen_t clientAddrLen = sizeof(clientAddr);
            int newsockfd = accept(sockfd, (struct sockaddr*)&clientAddr, &clientAddrLen);
            if (newsockfd == -1) {
                std::cerr << "无法接受客户端连接。" << std::endl;
                continue;
            }
            else{
                std::cerr << "客户端连接连接成功。" << std::endl;
            }

            // 创建一个新线程来处理客户端连接
            std::thread clientThread(handleClient, newsockfd);
            //接受客户端发送来的消息

        }

        // 主循环
        while (true) {
            sting = mod;
            // 在匿名管道中接收输出流（死鸽子暂时先不写）
            // ...

            // 在TCP中接收内容
            // ...

            // 对TCP内容进行加密
            // ...
    
            // 如果TCP接收到内容为"GetLog"，则返回lastest.ini内容
            // ...
    
            // 如果TCP接收到内容为"创建(.参数[文本型])"，则创建文件
            // ...
    
            // 如果TCP接收到内容为"修改(.参数一[文本型],参数二[文本型])"，则修改文件内容
            // ...

            // 启动BDS
            // ...
            // 输出选项
            cout << "请输入数字以启动相应功能" << endl;
            cout << "请输入：" << endl;
            std::cout << "1)启动BDS。" << endl;
            std::cout << "2)更新BDS。" << endl;
            std::cout << "3)推出程序。" << endl;
            std::cout << "请输入：" << endl;

            //模式选择&运行

            std::cin >> mod;
            if (mod == 1){
                std::string bdsFileName = "bedrocksserver"; // BDS可执行文件名

                 // 获取当前目录路径
                 fs::path currentPath = fs::current_path();

                 // 遍历当前目录
                for (const auto& entry : fs::directory_iterator(currentPath)) {
                    if (entry.is_regular_file() && entry.path().filename() == bdsFileName) {
                        // 找到了BDS可执行文件，执行BDS

                        // 获取BDS可执行文件的绝对路径
                        std::string bdsPath = entry.path().string();

                        // 使用system函数执行BDS命令
                        int result = std::system(LD_LIBRARY_PATH=. ./bedrock_server);

                        // 检查BDS是否启动成功
                        if (result == 0) {
                            std::cout << "BDS启动成功。" << std::endl;
                            std::cout << "等待1分钟，检测bedrockserver是否在运行..." << std::endl;
                            std::this_thread::sleep_for(std::chrono::minutes(1));

                            if (isBdsRunning(bdsFileName)) {
                                std::cout << "bedrockserver正在运行" << std::endl;
                            } else {
                                std::cout << "bedrockserver未在运行" << std::endl;
                                return 0;
                            }
                        }
                         else {
                            std::cerr << "BDS启动失败。" << std::endl;
                            return 1;
                        }

                        // 找到并启动BDS后，退出程序
                                    return 0;
                    }
                }

                // 未找到BDS可执行文件
                std::cerr << "未找到BDS可执行文件。" << std::endl;
                return 0;
            }
            else if(mod == 2){
                // 删除BDS文件夹中的bedrockserver文件
                fs::path bdsFilePath = bdsDir / bdsFileName;
                if (fs::exists(bdsFilePath)) {
                    if (!deleteFile(bdsFilePath)) {
                        return 1;
                    }
                }

                // 创建download文件夹
                if (!fs::exists(downloadDir) && !fs::create_directory(downloadDir)) {
                    std::cerr << "无法创建download文件夹。" << std::endl;
                    return 1;
                }

                // 下载最新的bedrockserver文件到download文件夹
                if (!downloadFile(downloadUrl, zipFilePath)) {
                    return 1;
                }

                // 解压下载的文件
                if (!unzipFile(zipFilePath, downloadDir)) {
                    return 1;
                }

                // 将下载的bedrockserver文件替换BDS文件夹中的bedrockserver文件
                fs::path downloadedBdsFilePath = downloadDir / bdsFileName;
                try {
                    fs::rename(downloadedBdsFilePath, bdsFilePath);
                    std::cout << "更新成功，BDS已更新至最新版本。" << std::endl;
                } catch (const fs::filesystem_error& ex) {
                    std::cerr << "无法替换bedrockserver文件：" << ex.what() << std::endl;
                    return 1;
                }
                return 0;
            }
            else if(mod ==3){
                 cout<<"请确认是否"<< endl;
                cout<<""<<endl;
                if (ask!="yes")
                {
                    run = false;
                }
                else {
                    return 0;
                }
            }
        }

    }  
    // 清理资源
    // ...
    close(sockfd); // 关闭服务器Socket
    clientThread.detach(); // 线程分离，使其在结束时自行清理资源
    cout<<"感谢使用ZEROCLEAR。";
    return 0;
}
#include "requestHandler.hpp"

const std::string FILE_MANAGER_NAME = "FileManager.sqlite3";
const std::string FILE_TREE_NAME    = "FileTree.tree";


bool emailInDB(const char email[], SQL_DB& loginDB) {
    loginDB.lock();
    loginDB.createStatement(
        std::string("select count(email) from login where email = \'") + email + "\';"
    );
    if (loginDB.nextRow()) {
        int64_t x = loginDB.extract<int64_t>(0);
        loginDB.unlock();
        return static_cast<bool>(x);
    }
    loginDB.unlock();
    throw std::ios::failure("Problematic SQL communcations for email acquisition");
}

std::pair<std::unique_ptr<Request::UserInfo>, ServerMessage> Request::signIn(
    const char email[], 
    const char pass[], 
    SQL_DB& loginDB, 
    BotWrapper& discord
) {
    // Nothing here should throw, beside the ios::failure, so the mutex should not end in an invalid state
    dpp::snowflake res = 0;
    try {
        bool used = emailInDB(email, loginDB);
        if (!used) {
            return {nullptr, {ServerMessage::Error, ServerMessage::WrongLogin}};
        }
        loginDB.lock();
        loginDB.createStatement(
            std::string("select password, salt, files_id from login where email = \'") + email + "\';"
        );
        if (!loginDB.nextRow()) {
            throw std::ios::failure("Invalid SQL state, no password found");
        }
        std::string password = loginDB.extract<std::string>(0);
        std::string salt     = loginDB.extract<std::string>(1);
        res = std::stoull(loginDB.extract<std::string>(2));
        loginDB.unlock();
        std::cout << "File snowflake is: " << res << std::endl;
        if (password != passwordHash(std::string(pass) + salt)) {
            return {nullptr, {ServerMessage::Error, ServerMessage::WrongLogin}};
        }
    }
    catch (std::exception& e) {
        loginDB.unlock();
        std::cerr << e.what() << std::endl;
        return {nullptr, {ServerMessage::Error, ServerMessage::InternalError}};
    }
    std::cout << "Logged in" << std::endl;
    std::vector<BotWrapper::File> contents = discord.download(res, discord.channel(BotWrapper::USER_INFO));
    std::string dbData = "", treeData = "";
    for (BotWrapper::File& file : contents) {
        if (file.name == FILE_MANAGER_NAME) {
            dbData = std::move(file.body);
        }
        else if (file.name == FILE_TREE_NAME) {
            treeData = std::move(file.body);
        }
    } 
    std::string dbFile = generateFilename(Files::SQL_APPEND);
    std::ofstream dbOut(dbFile, std::ios::trunc | std::ios::binary);
    if (dbOut.is_open() == false) {
        std::cerr << "Could not open user's db file" << std::endl;
        return {nullptr, {ServerMessage::Error, ServerMessage::InternalError}};
    }
    dbOut.write(dbData.c_str(), dbData.size());
    std::unique_ptr<UserInfo> info = std::make_unique<UserInfo>(UserInfo{
        email,
        res,
        SQL_DB(dbFile),
        std::move(treeData)
    });
    info->db.lock();
    if (!info->db.createStatement(
        "CREATE TABLE IF NOT EXISTS info ("
            "file TEXT PRIMARY KEY,"
            "data TEXT NOT NULL"
        ");"
    )) {
        std::cerr << "Statement of table 'info' creation failed" << std::endl;
    }
    info->db.nextRow();
    info->db.unlock();
    loggedMutex.lock();
    if (loggedEmails.contains(email)) {
        loggedMutex.unlock();
        return {nullptr, {ServerMessage::Error, ServerMessage::EmailAlreadyInUse, 0}};
    }
    loggedEmails.insert(email);
    loggedMutex.unlock();
    std::cout << "Successful sign in" << std::endl;
    return {std::move(info), {ServerMessage::OK, ServerMessage::NoError, 0}};
}

std::string generateSignUpCode() {
    static std::random_device rd;
    static std::seed_seq ss = { rd(), rd(), rd(), rd() };
    static std::mt19937 rng(ss);
    static std::uniform_int_distribution<char> uid('0', '9');
    static std::mutex randMutex;
    std::lock_guard<std::mutex> lock(randMutex);
    std::string code(6, '\0');
    for (char& ch : code) {
        ch = uid(rng);
    }
    return code;
}

// OK, code
std::pair<ServerMessage, std::string> Request::sendSignUpEmail(const char email[], const std::unordered_map<std::string, std::string>& secrets) {
    ServerMessage serverMsg = {ServerMessage::Error, ServerMessage::InternalError};
    const std::string sender_email = secrets.at(Secrets::EMAIL);
    const std::string sender_pass  = secrets.at(Secrets::EMAIL_PASSWORD);
    const std::string receiver_email = email;
    const std::string receiver_name  = receiver_email.substr(0, receiver_email.find_first_of('@'));
    const std::string code = generateSignUpCode();
    const std::string HOSTNAME = "smtp.gmail.com";
    const std::string SENDER_NAME = "RC Cloud Drive";
    std::cout << "\'" + sender_email + "\' " + "\'" + sender_pass + "\'" << std::endl;
    using namespace jed_utils::cpp;
    try {
        OpportunisticSecureSMTPClient client(HOSTNAME, 587);
        client.setCredentials(Credential(sender_email, sender_pass));
        PlaintextMessage msg(
            MessageAddress(sender_email, SENDER_NAME), 
            {
                MessageAddress(receiver_email, receiver_name)
            },
            "RC Cloud Drive : Sign-Up Authentication e-mail",
            "Welcome, " + receiver_name + "!" + "\r\n\r\n" + "Someone, hopefully you, has signed up to RC Cloud Drive using this e-mail."
                + "\r\n\r\n" + "If this was you, please enter the following code in the app's client:\r\n" + code + "\r\n"
                + "If not, we are sorry for the inconvenience" + "\r\n\r\n" + "This email has been generated automatically. Please do not respond to this email."
        );
        int err_no = client.sendMail(msg);
        if (err_no != 0) {
            std::cerr << client.getCommunicationLog() << '\n';
            std::cerr << "The error occured: " + client.getErrorMessage(err_no) << '\n';
            return {serverMsg, ""};
        }
        std::cout << "Send email operation completed!" << std::endl;
    }
    catch (std::exception& err) {
        std::cerr << err.what() << std::endl;
        return {serverMsg, ""};
    }
    serverMsg.type = ServerMessage::RequestCode;
    serverMsg.error = ServerMessage::NoError;
    return {serverMsg, code};
}

ServerMessage Request::verifyEmailAlreadyInUse(SQL_DB& loginDB, const char email[]) {
    try {
        if (emailInDB(email, loginDB)) {
            return {ServerMessage::Error, ServerMessage::EmailAlreadyInUse};
        }
        else {
            return {ServerMessage::OK, ServerMessage::NoError};
        }    
    }
    catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
        return {ServerMessage::Error, ServerMessage::InternalError};
    }
}

ServerMessage Request::finalizeSignup(
    const char* email, 
    const char* password, 
    BotWrapper& discord, 
    SQL_DB& loginDB
) {
    dpp::snowflake messageSnowflake = discord.upload(
        discord.channel(BotWrapper::USER_INFO), 
        {
            {FILE_MANAGER_NAME, ""},
            {FILE_TREE_NAME, std::string(1, '\0')}
        }
    );
    const std::string salt = generateSalt();
    const std::string hashedPass = passwordHash(password + salt);
    loginDB.lock();
    loginDB.createStatement(
        std::string("INSERT INTO login (email, password, salt, files_id) ") +
        "VALUES(" + "\'" + email + "\', \'" + hashedPass + "\', \'" + salt + "\', \'" + messageSnowflake.str() + "\');"
    );
    if (!loginDB.nextRow()) {
        loginDB.unlock();
        std::cerr << "Malformed sql statement" << std::endl;
        return {ServerMessage::Error, ServerMessage::InternalError};
    }
    loginDB.unlock();
    return {ServerMessage::OK, ServerMessage::NoError};
}

bool Request::sendTreeFile(const int client, const std::string& encoding) {
    ServerMessage smsg;
    smsg.type = ServerMessage::FileSend;
    smsg.content.size = encoding.size();
    if (Communication::write(client, &smsg, sizeof(smsg)) == false) {
        perror("error using write");
        return false;
    }
    if (Communication::write(client, encoding.c_str(), encoding.size()) == false) {
        perror("error writing file tree");
        return false;
    }
    // I don't care or want client confirmation
    return true;
}

void Request::updateDiscord(UserInfo& info, SQL_DB& loginDB, BotWrapper& discord) {
    dpp::snowflake updateSf = discord.upload(
        discord.channel(BotWrapper::USER_INFO),
        {
            {FILE_MANAGER_NAME, dpp::utility::read_file(info.db.name())},
            {FILE_TREE_NAME, info.tree}
        }
    );
    // update login db
    loginDB.lock();
    if (loginDB.createStatement(
        std::string("UPDATE login ") +
        "SET files_id = \'" + updateSf.str() + "\' " +
        "WHERE files_id = \'" + info.managerFile.str() + "\';"
    ) == false) {
        std::cerr << "Failure to create update" << std::endl;
        loginDB.unlock();
        return;
    }
    if (loginDB.nextRow() == false) {
        std::cerr << "Failure to update db" << std::endl;
        loginDB.unlock();
        return;
    }
    loginDB.unlock();
    // delete old files
    discord.remove(info.managerFile, discord.channel(BotWrapper::USER_INFO));
}

bool Request::deleteFile(int client, const std::string& id, SQL_DB& userDB, BotWrapper& discord) {
    userDB.lock();
    if (userDB.createStatement(
        "SELECT data FROM info WHERE file = \'" + id + "\';"
    ) == false) {
        std::cerr << "Invalid SQL statement" << std::endl;
    }
    if (userDB.nextRow() == false) {
        std::cerr << "No such file" << std::endl;
    }
    bool ok = true;
    std::stringstream ss(userDB.extract<std::string>(0));
    userDB.unlock();
    while (ss.good()) {
        uint64_t snowflake = 0;
        ss >> snowflake;
        if (discord.remove(snowflake, discord.channel(BotWrapper::DATA)) == false) {
            ok = false;
            std::cerr << "Failed to delete file" << std::endl;
        }
    }
    if (!ok) {
        return false;
    }
    userDB.lock();
    if (userDB.createStatement(
        "DELETE FROM info WHERE file = \'" + id + "\'"
    ) == false) {
        std::cerr << "Failure creating delete statement" << std::endl;
    }
    userDB.nextRow();
    userDB.unlock();
    return true;
}


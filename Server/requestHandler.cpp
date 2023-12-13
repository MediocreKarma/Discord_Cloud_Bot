#include "requestHandler.hpp"

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

std::pair<dpp::snowflake, ServerMessage> Request::signIn(const char email[], const char pass[], SQL_DB& loginDB) {
    // Nothing here should throw, beside the ios::failure, so the mutex should not end in an invalid state
    try {
        bool used = emailInDB(email, loginDB);
        if (!used) {
            return {0, {ServerMessage::Error, ServerMessage::WrongLogin}};
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
        dpp::snowflake res   = std::stoull(loginDB.extract<std::string>(2));
        loginDB.unlock();
        if (password != passwordHash(std::string(pass) + salt)) {
            return {0, {ServerMessage::Error, ServerMessage::WrongLogin}};
        }
        return {res, {ServerMessage::OK, ServerMessage::NoError}};
    }
    catch (std::exception& e) {
        loginDB.unlock();
        std::cerr << e.what() << std::endl;
        return {0, {ServerMessage::Error, ServerMessage::InternalError}};
    }
}

std::string generateSignUpCode() {
    static std::random_device rd;
    static std::seed_seq ss = { rd(), rd(), rd(), rd() };
    static std::mt19937 rng(ss);
    static std::uniform_int_distribution<char> uid('0', '9');
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
        bool used = emailInDB(email, loginDB);
        if (used) {
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
    dpp::cluster& discord, 
    SQL_DB& loginDB, 
    const dpp::snowflake clientInfoChannelSnowflake
) {
    dpp::message managerCreatorFile;
    managerCreatorFile
        .set_channel_id(clientInfoChannelSnowflake)
        .add_file(
            "FileManager.sqlite3", "" 
        ).add_file(
            "FileTree.tree", "\0"
        );
    dpp::snowflake messageSnowflake = discord.message_create_sync(managerCreatorFile).id;
    
    
    const std::string salt = generateSalt();
    const std::string hashedPass = passwordHash(password + salt);
    loginDB.lock();
    loginDB.createStatement(
        std::string("INSERT INTO login (email, password, salt, files_id) ") +
        "VALUES(" + "\'" + email + "\', \'" + hashedPass + "\', \'" + salt + "\', " + messageSnowflake.str() + ");"
    );
    if (!loginDB.nextRow()) {
        loginDB.unlock();
        std::cerr << "Malformed sql statement" << std::endl;
        return {ServerMessage::Error, ServerMessage::InternalError};
    }
    loginDB.unlock();
    return {ServerMessage::OK, ServerMessage::NoError};
}

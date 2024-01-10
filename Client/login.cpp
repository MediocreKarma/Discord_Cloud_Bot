#include "login.hpp"

static TextBox emailTextbox;
static TextBox passwordTextbox;
static TextBox confirmationTextbox;
static RoundedRectangleTextButton enterButton;
static RoundedRectangleTextShape errorOutput;
static RoundedRectangleTextButton switchUpToIn;
static RoundedRectangleTextButton switchInToUp;
static sf::Text titleText;

void draw(sf::RenderWindow& window, const std::vector<std::reference_wrapper<sf::Drawable>>& drawables) {
    auto clockStart = std::chrono::steady_clock::now();
    window.clear(sf::Color::White);
    for (auto& dable : drawables) {
        window.draw(dable);
    }
    window.display();
    auto clockEnd = std::chrono::steady_clock::now();
    auto dur = std::chrono::duration_cast<std::chrono::milliseconds>(clockEnd - clockStart);
    if (dur.count() < 33) {
        std::this_thread::sleep_for(std::chrono::milliseconds(33) - dur);
    }
}

LoginScreen::UserInfoData signUp(sf::RenderWindow& window);

void init(const sf::RenderWindow& window) {
    static bool initialised = false;
    if (!initialised) {
        titleText = sf::Text("My Cloud Drive", GUI::Font, 75);
        errorOutput = RoundedRectangleTextShape(sf::RoundedRectangleShape({1000, 0}, 0, 0), sf::Text("", GUI::Font));
        emailTextbox = TextBox(sf::RoundedRectangleShape({window.getSize().x / 2.f, 100}, 30, 30), GUI::Font, "Enter your email", 30, Colors::DarkGray, false, '\0', 64);
        passwordTextbox = TextBox(sf::RoundedRectangleShape({window.getSize().x / 2.f, 100}, 30, 30), GUI::Font, "Enter your password", 30, Colors::DarkGray, false, '*', 64);
        confirmationTextbox = TextBox(sf::RoundedRectangleShape({window.getSize().x / 2.f, 100}, 30, 30), GUI::Font, "Re-enter your password", 30, Colors::DarkGray, false, '*', 64);
        enterButton = RoundedRectangleTextButton(sf::RoundedRectangleShape({window.getSize().x / 5.f, 120}, 30, 30), sf::Text("Continue", GUI::Font, 45));
        switchUpToIn = RoundedRectangleTextButton(sf::RoundedRectangleShape({600, 50}, 0, 1), sf::Text("Already have an account? Sign-in here!", GUI::Font), true);
        switchInToUp = RoundedRectangleTextButton(sf::RoundedRectangleShape({500, 50}, 0, 1), sf::Text("New user? Sign-up instead!", GUI::Font), true);
        emailTextbox.setShapeFillColor(Colors::LightGray);
        passwordTextbox.setShapeFillColor(Colors::LightGray);
        confirmationTextbox.setShapeFillColor(Colors::LightGray);
        enterButton.setShapeFillColor(sf::Color::Green);
        errorOutput.setShapeFillColor(sf::Color::Transparent);
        errorOutput.setShapeOutlineColor(sf::Color::Transparent);
        errorOutput.setTextFillColor(sf::Color::Red);
        switchUpToIn.setShapeFillColor(sf::Color::Transparent);
        switchUpToIn.setShapeOutlineColor(sf::Color::Transparent);
        switchUpToIn.setTextFillColor(Colors::DarkGray);
        switchUpToIn.setStyle(sf::Text::Style::Underlined);
        switchInToUp.setShapeFillColor(sf::Color::Transparent);
        switchInToUp.setShapeOutlineColor(sf::Color::Transparent);
        switchInToUp.setTextFillColor(Colors::DarkGray);
        switchInToUp.setStyle(sf::Text::Style::Underlined);
        titleText.setFillColor(sf::Color::Black);
    }
    initialised = true;
    // emailTextbox.replaceShape(sf::RoundedRectangleShape({window.getSize().x / 2.f, 100}, 30, 30));
    // passwordTextbox.replaceShape(sf::RoundedRectangleShape({window.getSize().x / 2.f, 100}, 30, 30));
    // confirmationTextbox.replaceShape(sf::RoundedRectangleShape({window.getSize().x / 2.f, 100}, 30, 30));
    // enterButton.replaceShape(sf::RoundedRectangleShape({window.getSize().x / 5.f, 120}, 30, 30));
    // switchUpToIn.replaceShape(sf::RoundedRectangleShape({540, 50}, 0, 1));
    // switchInToUp.replaceShape(sf::RoundedRectangleShape({400, 50}, 0, 1));
    sf::FloatRect fr = titleText.getLocalBounds();
    titleText.setOrigin(fr.width / 2 + fr.left, fr.top);
    titleText.setPosition(window.getSize().x / 2, window.getSize().y / 6);
    emailTextbox.setOrigin(
        emailTextbox.getSize().x / 2,
        emailTextbox.getSize().y / 2  
    );
    passwordTextbox.setOrigin(
        passwordTextbox.getSize().x / 2,
        passwordTextbox.getSize().y / 2
    );
    confirmationTextbox.setOrigin(
        confirmationTextbox.getSize().x / 2,
        confirmationTextbox.getSize().y / 2
    );
    enterButton.setOrigin(
        enterButton.getSize().x / 2,
        enterButton.getSize().y / 2
    );
    switchUpToIn.setOrigin(
        switchUpToIn.getSize().x / 2,
        switchUpToIn.getSize().y / 2
    );
    switchInToUp.setOrigin(
        switchInToUp.getSize().x / 2,
        switchInToUp.getSize().y / 2
    );
    emailTextbox.setPosition(
        window.getSize().x / 2, 
        window.getSize().y / 2 - 100
    );
    passwordTextbox.setPosition(
        emailTextbox.getPosition().x,
        emailTextbox.getPosition().y + 120
    );
    switchInToUp.setPosition(
        window.getSize().x / 2 - 200,
        passwordTextbox.getPosition().y + 80
    );
    confirmationTextbox.setPosition(
        emailTextbox.getPosition().x,
        passwordTextbox.getPosition().y + 120
    );
    switchUpToIn.setPosition(
        window.getSize().x / 2 - 200,
        confirmationTextbox.getPosition().y + 80
    );
    errorOutput.setPosition(
        window.getSize().x / 2 - errorOutput.getSize().x / 2,
        emailTextbox.getPosition().y - 100
    );
}

LoginScreen::UserInfoData getUserInfo(sf::RenderWindow& window, const bool wrongLogin = false, const bool wrongSignup = false) {
    init(window);
    if (wrongLogin) {
        errorOutput.setString("You have entered an invalid username or password");
    }
    if (wrongSignup) {
        errorOutput.setString("Invalid or already used email");
        return signUp(window);
    }
    enterButton.setPosition(
        window.getSize().x / 2,
        passwordTextbox.getPosition().y + 200
    );  
    std::vector<std::reference_wrapper<sf::Drawable>> drawables = {
        std::ref(emailTextbox), 
        std::ref(passwordTextbox),
        std::ref(enterButton),
        std::ref(switchInToUp),
        std::ref(errorOutput),
        std::ref(titleText)
    };
    std::string email = emailTextbox.getData();
    std::string password = passwordTextbox.getData();
    sf::Vector2i mouse = {-1, -1};
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            switch (event.type) {
                case sf::Event::Resized:
                    window.setView(sf::View(sf::FloatRect(0, 0, event.size.width, event.size.height)));
                    return getUserInfo(window, wrongLogin, wrongSignup);
                case sf::Event::Closed:
                    window.close(); 
                    return {"", "", 0};
                case sf::Event::MouseButtonPressed:
                    if (event.mouseButton.button != sf::Mouse::Left) {
                        break;
                    }
                    mouse = {event.mouseButton.x, event.mouseButton.y};
                    break;
            }
            while (mouse.x != -1) {
                sf::Vector2i newMouse = {-1, -1};
                bool enterTransition = false;
                if (emailTextbox.Hittable::hit(mouse.x, mouse.y)) {
                    email = emailTextbox.getDataFromInput(window, mouse.x, mouse.y);
                    if (emailTextbox.reqRefresh()) {

                    }
                    if (emailTextbox.pressedEnter() == false) {
                        newMouse = emailTextbox.lastMouseInput();
                    }
                    else {
                        enterTransition = true;
                    }
                }
                if (enterTransition || passwordTextbox.Hittable::hit(mouse.x, mouse.y)) {
                    enterTransition = false;
                    password = passwordTextbox.getDataFromInput(window, mouse.x, mouse.y);
                    if (passwordTextbox.pressedEnter() == false) {
                        newMouse = passwordTextbox.lastMouseInput();
                    }
                    else {
                        enterTransition = true;
                    }
                }
                if (enterTransition || enterButton.Hittable::hit(mouse.x, mouse.y)) {
                    enterTransition = false;
                    if (emailTextbox.hasUserInput() && passwordTextbox.hasUserInput()) {
                        errorOutput.setString("");
                        return {email, password, 0};
                    }
                    else {
                        errorOutput.setString("Please complete all fields");
                    }
                }
                if (switchInToUp.Hittable::hit(mouse.x, mouse.y)) {
                    return signUp(window);
                }
                mouse = newMouse;
            }
        }
        draw(window, drawables);
    }
    return LoginScreen::UserInfoData();
}

LoginScreen::UserInfoData signUp(sf::RenderWindow& window) {
    init(window);
    enterButton.setPosition(
        window.getSize().x / 2,
        confirmationTextbox.getPosition().y + 200
    );
    std::vector<std::reference_wrapper<sf::Drawable>> drawables = {
        std::ref(emailTextbox), 
        std::ref(passwordTextbox),
        std::ref(confirmationTextbox),
        std::ref(enterButton),
        std::ref(switchUpToIn),
        std::ref(errorOutput),
        std::ref(titleText)
    };
    std::string email = emailTextbox.getData();
    std::string password = passwordTextbox.getData();
    std::string confirmationPassword = confirmationTextbox.getData();
    sf::Vector2i mouse = {-1, -1};
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            switch (event.type) {
                case sf::Event::Closed:
                    return {"", "", 0}; 
                    break;
                case sf::Event::Resized:
                    window.setView(sf::View(sf::FloatRect(0, 0, event.size.width, event.size.height)));
                    return signUp(window);
                case sf::Event::MouseButtonPressed:
                    if (event.mouseButton.button != sf::Mouse::Left) {
                        break;
                    }
                    mouse = {event.mouseButton.x, event.mouseButton.y};
                    break;
            }
            while (mouse.x != -1) {
                sf::Vector2i newMouse = {-1, -1};
                bool enterTransition = false;
                if (emailTextbox.Hittable::hit(mouse.x, mouse.y)) {
                    email = emailTextbox.getDataFromInput(window, mouse.x, mouse.y);
                    if (emailTextbox.pressedEnter() == false) {
                        newMouse = emailTextbox.lastMouseInput();
                    }
                    else {
                        enterTransition = true;
                    }
                }
                if (enterTransition || passwordTextbox.Hittable::hit(mouse.x, mouse.y)) {
                    enterTransition = false;
                    password = passwordTextbox.getDataFromInput(window, mouse.x, mouse.y);
                    if (passwordTextbox.pressedEnter() == false) {
                        newMouse = passwordTextbox.lastMouseInput();
                    }
                    else {
                        enterTransition = true;
                    }
                }
                if (enterTransition || confirmationTextbox.Hittable::hit(mouse.x, mouse.y)) {
                    enterTransition = false;
                    confirmationPassword = confirmationTextbox.getDataFromInput(window, mouse.x, mouse.y);
                    if (confirmationTextbox.pressedEnter() == false) {
                        newMouse = confirmationTextbox.lastMouseInput();
                    }
                    else {
                        enterTransition = true;
                    }
                }
                if (enterTransition || enterButton.Hittable::hit(mouse.x, mouse.y)) {
                    enterTransition = false;
                    if (emailTextbox.hasUserInput() && passwordTextbox.hasUserInput() && confirmationTextbox.hasUserInput()) {
                        if (password == confirmationPassword) {
                            errorOutput.setString("");
                            return {email, password, 1};
                        }
                        errorOutput.setString("The passwords do not match");
                    }
                    else {
                        errorOutput.setString("Please complete all fields");
                    }
                }
                if (switchUpToIn.Hittable::hit(mouse.x, mouse.y)) {
                    return getUserInfo(window);
                }
                mouse = newMouse;
            }
        }
        draw(window, drawables);
    }
    return LoginScreen::UserInfoData();
}

std::string getEmailConfirmationCode(sf::RenderWindow& window, bool wrongCodeEntered = false) {
    init(window);
    if (wrongCodeEntered) {
        errorOutput.setString("You have entered the wrong code");
    }
    TextBox codeTextbox(sf::RoundedRectangleShape({250, 75}, 15, 30), GUI::Font, "000000", 60, Colors::DarkGray, false, '\0', 6);
    codeTextbox.setOrigin(codeTextbox.getSize().x / 2, codeTextbox.getSize().y / 2);
    codeTextbox.setPosition(
        window.getSize().x / 2, 
        window.getSize().y / 2 - 100
    );
    //codeTextbox.setScale(2, 2);
    codeTextbox.setShapeFillColor(Colors::LightGray);
    const char alphabet[] = "0123456789";
    codeTextbox.setAlphabet(std::unordered_set<char>(alphabet, alphabet + sizeof(alphabet) / sizeof(char) - 1));
    RoundedRectangleTextShape infoShape(sf::RoundedRectangleShape({0, 0}, 0, 0), sf::Text("Please be patient, the confirmation email\nmay take a few moments before arriving.", GUI::Font));
    infoShape.setOrigin(infoShape.getSize().x / 2, infoShape.getSize().y / 2);
    infoShape.setPosition(
        window.getSize().x / 2,
        codeTextbox.getPosition().y + 100
    );
    infoShape.setShapeFillColor(sf::Color::Transparent);
    infoShape.setShapeOutlineColor(sf::Color::Black);
    enterButton.setPosition(
        window.getSize().x / 2,
        infoShape.getPosition().y + 200
    );
    std::vector<std::reference_wrapper<sf::Drawable>> drawables = {
        std::ref(codeTextbox),
        std::ref(infoShape),
        std::ref(errorOutput),
        std::ref(enterButton),
        std::ref(titleText)
    };
    std::string code;
    sf::Vector2i mouse = {-1, -1};
    while (window.isOpen()) {
        sf::Event event;
        bool enterTransition = false;
        while (window.pollEvent(event)) {
            switch (event.type) {
                case sf::Event::Closed:
                    window.close(); 
                    return "";
                case sf::Event::Resized:
                    window.setView(sf::View(sf::FloatRect(0, 0, event.size.width, event.size.height)));
                    return getEmailConfirmationCode(window, wrongCodeEntered);
                case sf::Event::TextEntered:
                    if (event.text.unicode < 128 && event.text.unicode != '\r') {
                        code = codeTextbox.externalType(window, event.text.unicode);
                        if (codeTextbox.pressedEnter()) {
                            enterTransition = true;
                        }
                        else {
                            mouse = codeTextbox.lastMouseInput();
                        }
                    }
                    break;
                case sf::Event::MouseButtonPressed:
                    if (event.mouseButton.button != sf::Mouse::Left) {
                        break;
                    }
                    mouse = {event.mouseButton.x, event.mouseButton.y};
                    break;
            }
            while (mouse.x != -1 || enterTransition) {
                sf::Vector2i newMouse = {-1, -1};
                if (codeTextbox.Hittable::hit(mouse.x, mouse.y)) {
                    code = codeTextbox.getDataFromInput(window, mouse.x, mouse.y);
                    if (codeTextbox.pressedEnter() == false) {
                        newMouse = codeTextbox.lastMouseInput();
                    }
                    else {
                        enterTransition = true;
                    }
                }
                if (enterTransition || enterButton.Hittable::hit(mouse.x, mouse.y)) {
                    enterTransition = false;
                    if (code.size() == 6) {
                        errorOutput.setString("");
                        return code;
                    }
                    else {
                        errorOutput.setString("The code should have 6 digits!");
                    }
                }
                mouse = newMouse;
            }
        }
        draw(window, drawables);
    }

    return "";
}

bool LoginScreen::loginProcedure(sf::RenderWindow& window, const int sd) {
    ClientMessage cmessage;
    memset(&cmessage, 0, sizeof(cmessage));
    ServerMessage smessage;
    memset(&smessage, 0, sizeof(smessage));
    bool wrongLogin = false;
    bool wrongSignup = false;
    while (true) {
        auto [email, password, signup] = getUserInfo(window, wrongLogin, wrongSignup);
        if (!window.isOpen()) {
            return false;
        }
        loadingMessage(window);
        strncpy(cmessage.content.signData.email,   email.c_str(), 64);
        strncpy(cmessage.content.signData.pass, password.c_str(), 64);
        std::cout << "\'" << email << "\' \'" << password << "\'" << std::endl;
        if (signup) {
            cmessage.type = ClientMessage::SignUpRequest;
        } 
        else {
            cmessage.type = ClientMessage::SignInRequest;
        }
        if (Communication::write(sd, &cmessage, sizeof(cmessage)) == false) {
            perror("Write to server failed, connection is dead");
            return false;
        }
        std::cout << "Wrote message to server" << std::endl;
        if (Communication::read(sd, &smessage, sizeof(smessage)) == false) {
            // unreachable error, if it occurs there is a critical issue
            perror("Error using read");
            return false;
        }
        std::cout << "Received response from server" << std::endl;
        if (signup) {
            if (smessage.type == ServerMessage::ServerQuit) {
                break;
            }
            else if (smessage.type == ServerMessage::Error && (smessage.error == ServerMessage::EmailAlreadyInUse || smessage.error == ServerMessage::InternalError)) {
                wrongSignup = true;
                wrongLogin  = false;
                continue;
            }
            else if (smessage.type == ServerMessage::Error) {
                std::cerr << ServerMessage::humanReadable(smessage.error) << std::endl;
                continue;
            }
            if (smessage.type != ServerMessage::RequestCode) {
                std::cerr << "Invalid transmission\n";
                return false;
            }
            cmessage.type = ClientMessage::SignUpCode;
            bool wrongCodeEntered = false;
            std::cout << "Preparing signup code for server: ";
            while (smessage.type != ServerMessage::OK) {
                // Getting email confirmation code
                std::string code = getEmailConfirmationCode(window, wrongCodeEntered);
                if (!window.isOpen()) {
                    return false;
                }
                std::cout << "\'" + code + "\'" << std::endl;
                strncpy(cmessage.content.signData.signCode, code.c_str(), 6);
                if (Communication::write(sd, &cmessage, sizeof(cmessage)) == -1) {
                    perror("Write to server failed, connection is dead");
                    return false;
                }
                std::cout << "Wrote code to server" << std::endl;
                if (Communication::read(sd, &smessage, sizeof(smessage)) == -1) {
                    // unreachable error, if it occurs there is a critical issue
                    perror("Error using read");
                    return false;
                }
            }
            // signup succesful
            std::cout << "Signup success!" << std::endl;
        }
        else {
            if (smessage.type == ServerMessage::ServerQuit) {
                break;
            }
            else if (smessage.type == ServerMessage::Error && smessage.error == ServerMessage::WrongLogin) {
                wrongLogin  = true;
                wrongSignup = false;
                continue;
            }
            // Server should not outright refuse login attempts
            else if (smessage.type == ServerMessage::Error) {
                std::cout << ServerMessage::humanReadable(smessage.error) << std::endl;
                continue;
            }
            else if (smessage.type != ServerMessage::OK) {
                std::cout << "Invalid transmission" << std::endl;
                return false;
            }
            std::cout << "Logged in succesfully" << std::endl;
            return true;
        }
    }
    return false;
}
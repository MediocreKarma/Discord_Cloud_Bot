In order to run and compile the project, the following dependencies are necessary:

    SQLite3: sudo apt install libsqlite3-dev
    DPP: https://dpp.dev/install-linux-deb.html
    OpenSSL: sudo apt install libssl-dev
    SFML: sudo apt install libsfml-dev

Afterwards, run:

cmake .
make


Additionally, in order to run the program, a special argument must be provided
to the server function, or placed in the `resources` sub-directory with the name
`secrets.config`. This file must be formatted in the following manner.
"
email=my.email.addres@abc.def
password=1111 2222 3333 4444
bot_token=YOUR BOT TOKEN
guild_snowflake=YOUR GUILD SNOWFLAKE
"

For gmail addresses, an app password is required.
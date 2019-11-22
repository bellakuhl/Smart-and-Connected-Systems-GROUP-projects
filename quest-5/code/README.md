# Security Central

There are three actors in this security central software package: the transmitter
(fob), the security hub, and the web server. Both the transmitter and the security
hub code are meant to be run on the appropriate hardware described in the
[report](../report.md).


## Transmitter

The transmitter code lives in the `transmitter` directory. To flash a new transmitter:

1) `cd transmitter`
2) `idf.py menuconig`
    a) Fob Config
    b) Set the fob id
    c) Set the fob code
    d) Save the configuration
3) `idf.py build && idf.py flash`


## Security Hub

1) `cd security_hub`
2) `idf.py menuconfig`
    a) Choose "Security Hub Config"
    b) Set WIFI_SSID and WIFI_PASSPHRASE
    c) Set SERVER_HOST and SERVER_PORT to the raspberry pi
    d) Set SERVER_USERNAME and SERVER_PASSWORD to the username and password
       setup for this security hub.
3) `idf.py build && idf.py flash`


## Web Server

All of the instructions in this section assume your starting directory is the `webserver/`
directory. All relative paths assume this starting point.

### Development

The web server is built on a stack of NodeJS libraries, and provides a front end
client built using React and other modern front end libraries. To get started
with development:

0. Make sure NodeJS >= 10.15.0 is installed
1. For the tools used in this repo, it will be hugely beneficial to install `npx`.
   `npx` provides an easy way to run cli programs shipped with NodeJS
   packages without having to figure out where they are in the local `node_modules`
   folder.
2. `cd server && npm install`
3. `cd client && npm install`

To run the code in development mode, it's probably best to start the typescript
and webpack compilers as watchers, so as you make changes to the code, the
javascript files will be automatically created.

For the server component:
* `cd server`
* `npx tsc --watch`

For the client:
* `cd client`
* `npx webpack --watch`

While the watchers are nice, they aren't fool proof. If you notice that changes
you are making are not reflected in the running code, try restarting the above
watchers.

> Microsoft's Visual Studio Code provides a nice environment for working on
> projects written in Typescript. It might be worth while to familiarize
> yourself with VSCode if you plan on working a lot with Typescript.


To start the web server in development:

0. `cd server`
1. `node index.js`

You MUST be in the server folder for the static server paths to work!

#### Mock Data

When working in development you may want to insert some mock data into the database
for being able to test fob access requests. There is a script included in the `server`
directory that will do this. Simply run: `node scripts/createUsers.js` from that directory
to populate the database with an admin user and a security hub user.


### Release & Deployment

Since the raspberry pi is a must more constrained environment, or simply so you don't
have to have all of the development dependencies on your production environment, there
is a script included to package an archive of code for deployment.

To build a release, from the `webserver` directory run: `./package.sh`

> The deployment script is  written in bash (sorry Windows users).

This will produce a file `security_central.zip`. To deploy the file, copy it
to a production machine and unzip the file.  It should produce the following folder
structure:

```
security_central/
    client/
        ...
    server/
        ...
```

To get things running:

1. `cd security_central/client`
2. `npm install --production`
3. `cd ../server`
4. `npm install --production`

Then start the web server: `npm index.js`

> The default port the server bind to is `8000`. You can pass a port number
> when starting `index.js` to change it: `node index.js <port>`


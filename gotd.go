package main

import (
	"bufio"
	"flag"
	"fmt"
	"io/ioutil"
	"log"
	"math/rand"
	"net"
	"os"
	"strings"
)

var (
	mottos   []string
	mottoCnt int
	logger   *log.Logger
	tracer   *log.Logger
)

var cfg = struct {
	bindIface   string
	bindPortTcp string
	bindPortUdp string
	mottoFile   string
	mottoDelim  string
	verbose     bool
}{
	bindIface:   "localhost",
	bindPortTcp: "1717",
	bindPortUdp: "1717",
	mottoFile:   "./motd.txt",
	mottoDelim:  "÷",
	verbose:     false,
}

func initialize() {
	flag.StringVar(&cfg.bindIface, "i", cfg.bindIface, "interface to bind to")
	flag.StringVar(&cfg.bindPortTcp, "p", cfg.bindPortTcp, "TCP port to listen on")
	flag.StringVar(&cfg.bindPortUdp, "u", cfg.bindPortUdp, "UDP port to listen on")
	flag.StringVar(&cfg.mottoFile, "m", cfg.mottoFile, "motto file")
	flag.StringVar(&cfg.mottoDelim, "d", cfg.mottoDelim, "delimiter used in motto file")
	flag.BoolVar(&cfg.verbose, "v", cfg.verbose, "produce verbose output")
	flag.Parse()
	if 0 != len(flag.Args()) {
		fmt.Println("unrecognized options: ", flag.Args())
		flag.Usage()
		os.Exit(1)
	}

	logger = log.New(os.Stderr, "", log.Ldate|log.Ltime|log.Lshortfile)
	if cfg.verbose {
		tracer = log.New(os.Stderr, "", log.Ldate|log.Ltime|log.Lshortfile)
	} else {
		tracer = log.New(ioutil.Discard, "", 0)
	}

	tracer.Print("interface:  ", cfg.bindIface)
	tracer.Print("TCP port:   ", cfg.bindPortTcp)
	tracer.Print("UDP port:   ", cfg.bindPortUdp)
	tracer.Print("motto file: ", cfg.mottoFile)
	tracer.Print("delimiter:  ", cfg.mottoDelim)
	tracer.Print("log sink:   ", cfg.verbose)
}

func check(err error, msg string) {
	if err != nil {
		logger.Print(msg, ": ", err.Error())
		os.Exit(1)
	}
}

func motto() string {
	return strings.TrimSpace(mottos[rand.Intn(mottoCnt)]) + "\n"
}

func serveTCP(sock net.Listener) {
	for {
		conn, err := sock.Accept()
		check(err, "Accept")
		tracer.Print("TCP connect from ", conn.RemoteAddr())
		// Since we never read from the socket we do this synchronously:
		conn.Write([]byte(motto()))
		conn.Close()
	}
}

func serveUDP(sock *net.UDPConn) {
	buf := make([]byte, 1024)
	for {
		size, raddr, err := sock.ReadFrom(buf)
		check(err, "sock.ReadFrom")
		tracer.Printf("UDP packet (%d bytes) from %s", size, raddr)
		sock.WriteTo([]byte(motto()), raddr)
	}
}

func main() {
	initialize()

	rawmot, err := ioutil.ReadFile(cfg.mottoFile)
	check(err, "ioutilTcp.ReadFile "+cfg.mottoFile)
	mottos = strings.Split(string(rawmot), cfg.mottoDelim)
	mottoCnt = len(mottos)
	tracer.Printf("have %d mottos", mottoCnt)

	bindaddr := cfg.bindIface + ":" + cfg.bindPortTcp
	tsock, err := net.Listen("tcp", bindaddr)
	check(err, "net.Listen tcp "+bindaddr)
	defer tsock.Close()
	tracer.Print("listening on TCP ", bindaddr)
	go serveTCP(tsock)

	bindaddr = cfg.bindIface + ":" + cfg.bindPortUdp
	laddr, err := net.ResolveUDPAddr("udp", bindaddr)
	check(err, "net.ResolveUDPAddr")
	usock, err := net.ListenUDP("udp", laddr)
	check(err, "net.ListenUDP")
	defer usock.Close()
	tracer.Print("listening on UDP ", bindaddr)
	go serveUDP(usock)

	// ready signal
	fmt.Println("")
	// exit on any input on stdin
	reader := bufio.NewReader(os.Stdin)
	reader.ReadRune()
}

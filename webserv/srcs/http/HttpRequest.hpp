#ifndef HTTPREQUEST_HPP
# define HTTPREQUEST_HPP

# include "../../libraries/includes.hpp"

class HttpRequest {
private:
	HttpRequest();
	HttpRequest(const HttpRequest& other);
	HttpRequest& operator=(const HttpRequest& other);
	enum httpType { REQUEST, INVALID };
	
	int			_DetermineHttpType(const std::string& httpMessage);
	void		_parseHttpRequest(const std::string& httpRequest, sConnection& client);
	void		_init_structures(sConnection &connection);
	int			_find_headers_limit(sConnection &connection, const char *buffer);
public:
	HttpRequest(sConnection& client);
	~HttpRequest();

};

class HttpParseException : public std::exception {
	public:
		virtual const char *what() const throw()
		{
			return (RED_TEXT "ERROR: HTTP PARSE ERROR" RESET_COLOR);
		}
};
class InvalidHttpMethodException : public std::exception {
	public:
		virtual const char *what() const throw()
		{
			return (RED_TEXT "ERROR: INVALID HTTP METHOD ERROR" RESET_COLOR);
		}
};
class SocketError : public std::exception {
	public:
		virtual const char *what() const throw()
		{
			return (RED_TEXT "RECV SOCKET ERROR" RESET_COLOR);
		}
};
class SocketClosed : public std::exception {
	public:
		virtual const char *what() const throw()
		{
			return (RED_TEXT "RECV SOCKET CLOSED" RESET_COLOR);
		}
};
class SizeExceeded : public std::exception {
	public:
		virtual const char *what() const throw()
		{
			return (RED_TEXT "MAX REQUEST SIZE EXCEEDED" RESET_COLOR);
		}
};

#endif

// valgrind --leak-check=full ./webserv configs/kevin.conf
// curl -X POST -F "file=@/nfs/homes/kzak/Desktop/web/prova.txt" localhost:8081/
// curl -X DELETE localhost:8080/upload/prova.txt
// curl -X PUT localhost:8080/upload/README.md 
// curl --resolve example.it:8080:127.0.0.1 http://example.it
// curl --resolve example.com:8080:127.0.0.1 http://example.com

// GET /favicon.ico HTTP/1.1
// Host: localhost:8090
// User-Agent: Mozilla/5.0 (X11; Ubuntu; Linux x86_64; rv:109.0) Gecko/20100101 Firefox/117.0
// Accept: image/avif,image/webp,*/*
// Accept-Language: en-US,en;q=0.5
// Accept-Encoding: gzip, deflate, br
// Connection: keep-alive
// Referer: http://localhost:8090/
// Sec-Fetch-Dest: image
// Sec-Fetch-Mode: no-cors
// Sec-Fetch-Site: same-origin

// GET /index.html HTTP/1.1
// Host: localhost:9000
// User-Agent: Mozilla/5.0 (X11; Ubuntu; Linux x86_64; rv:109.0) Gecko/20100101 Firefox/118.0
// Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,*/*;q=0.8
// Accept-Language: en-US,en;q=0.5
// Accept-Encoding: gzip, deflate, br
// Connection: keep-alive
// Upgrade-Insecure-Requests: 1
// Sec-Fetch-Dest: document
// Sec-Fetch-Mode: navigate
// Sec-Fetch-Site: none
// Sec-Fetch-User: ?1

// https://blog.codefarm.me/2021/09/24/http-redirect-3xx/

//---------------------------------------------------------------
// Nginx, essendo un server web e un proxy inverso,
// utilizza il multiplexing per gestire in modo efficiente
// più richieste da parte dei client. Due delle tecniche
// principali utilizzate da Nginx sono:

// Event-Driven Architecture: Nginx utilizza
// un'architettura basata sugli eventi (event-driven)
// per gestire le connessioni in modo efficiente.
// Piuttosto che utilizzare un thread separato per ogni
// connessione, Nginx utilizza un modello basato sugli
// eventi per gestire più connessioni simultaneamente
// con un numero limitato di thread.

// Asynchronous I/O: Nginx utilizza chiamate I/O
// asincrone per gestire le operazioni di lettura
// e scrittura senza bloccare il thread principale.
// Questo consente a Nginx di gestire molte connessioni
// contemporaneamente senza dover creare un thread
// separato per ciascuna connessione.

// Grazie a queste tecniche, Nginx è in grado di
// gestire un grande numero di connessioni concorrenti
// in modo efficiente, fornendo prestazioni elevate
// e una bassa latenza. Ciò è particolarmente importante
// in scenari ad alta concorrenza, come server web
// che devono gestire numerose richieste simultaneamente.

// Inoltre, Nginx può essere configurato per gestire
// il load balancing tra più server backend,
// distribuendo il carico di lavoro tra di essi.
// Questo aiuta ulteriormente a sfruttare in modo
// efficiente le risorse disponibili e garantire
// una maggiore affidabilità e scalabilità.


// Certamente, ecco una spiegazione dei principali valori
// restituiti da Siege con il comando `siege -c 10 -r 5 http://localhost:8080`:

// - **Transactions:** Il numero totale di richieste HTTP eseguite durante il test.

// - **Availability:** La percentuale di richieste riuscite
// rispetto al totale delle richieste inviate.
// Una percentuale del 100% indica che tutte le richieste sono state gestite con successo.

// - **Elapsed time:** Il tempo totale trascorso durante il test.

// - **Data transferred:** La quantità totale di dati trasferiti durante il test.

// - **Response time:** Il tempo medio impiegato per
// elaborare ciascuna richiesta. In questo caso, è di 0.02 secondi,
// indicando il tempo medio di risposta.

// - **Transaction rate:** Il numero medio di transazioni
// completate al secondo. Nel tuo caso, è di 500 transazioni al secondo.

// - **Throughput:** La quantità media di dati trasferiti
// al secondo. Nel tuo caso, è di 0.07 MB al secondo.

// - **Concurrency:** Il numero medio di utenti simultanei
// durante il test. Nel tuo caso, è di 9.00.

// - **Successful transactions:** Il numero totale di
// richieste HTTP gestite con successo.

// - **Failed transactions:** Il numero totale di richieste
// HTTP che hanno restituito un errore.

// - **Longest transaction:** Il tempo più lungo impiegato
// per completare una singola transazione.

// - **Shortest transaction:** Il tempo più breve impiegato
// per completare una singola transazione.

// In sintesi, questi valori forniscono una panoramica delle
// prestazioni del tuo server web durante il test di carico.
// Un'alta availability, un basso response time, e un elevato
// transaction rate sono generalmente indicatori positivi,
// ma è importante considerare tutti i valori insieme per
// ottenere una comprensione completa delle prestazioni.


//! installare siege

// wget https://download.joedog.org/siege/siege-latest.tar.gz

// tar -xzvf siege-latest.tar.gz

// cd siege-<versione> //! scrivetelo a mano

// ./configure --prefix=$HOME/siege
// make
// make install

// export PATH=$PATH:$HOME/siege/bin

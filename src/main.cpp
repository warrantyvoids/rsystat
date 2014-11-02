#include <iostream>
#include <curses.h>
#include <cmath>

#include "Connection.h"
#include "Configuration.h"

int main(int argc, char** argv) {
  Configuration conf;
  
  std::vector<Connection> connections;
  
  connections.reserve(conf.getHosts().size());

  for (std::string hostname : conf.getHosts()) {
    std::cout << hostname << std::endl;
    connections.emplace_back( hostname );
  }
  

  for (Connection& conn : connections) {
    if (!conn) {
      std::cerr << "Error!" << std::endl;
      continue;
    }
    OperatingSystem * os = conn.getOS();
    auto loads = os->getLoad();
    for (double load : loads) {
      std::cout << load << " ";
    }
    std::cout << std::endl;
  }

  setlocale(LC_ALL, "en_US.UTF-8");

  initscr();
  timeout(1000);
  int row = 1;
  attron(A_DIM);
  mvaddstr(0, 0, "Hostname");
  mvaddstr(0,40, "| Load");
  mvaddstr(0,60, "| #usr");
  attroff(A_DIM);
  start_color();
  init_pair(1, COLOR_RED, COLOR_BLACK);
  std::string progress = "|/-\\";
  int prog = 0;
  do {
    attron(A_STANDOUT);
    mvaddch(0, 79, progress[prog]);
    attroff(A_STANDOUT);
    for (Connection & conn : connections) {
      mvaddstr(row, 1, conn.getHostname().c_str());
    
      mvaddch(row, 40, '|');
      if (!conn) {
        attron(A_BLINK | COLOR_PAIR(1));
        addstr(" OFFLINE ");
        attroff(A_BLINK | COLOR_PAIR(1));
        continue;
      }
    
      std::array<double,3> load = conn.getOS()->getLoad();
      std::array<double,5> interp;
      interp[0] = load[0];
      interp[1] = (load[0] + load[1])/2;
      interp[2] = load[1];
      interp[3] = (load[1] + load[2])/2;
      interp[4] = load[2];
      cchar_t t;
      t.attr = 0;
      t.chars[1] = L'\0';
      for (double load : interp) {
        int loadScaled = (int)( std::log(load)/std::log(10) * 4 + 6 );
        loadScaled = loadScaled > 8 ? 8 : loadScaled;
        loadScaled = loadScaled < 0 ? 0 : loadScaled;
        t.chars[0] = L'\u2581'+ loadScaled ;
        add_wch( &t );
      }
      row++;
    }
    refresh();
    row = 1;
    
    prog++;
    if (prog == 4)
      prog = 0;
    
  } while (getch() != 'q');
  
  endwin();
  
  return 0;
}
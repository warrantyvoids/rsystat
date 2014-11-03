#include <iostream>
#include <curses.h>
#include <cmath>
#include <algorithm>

#include "Connection.h"
#include "Configuration.h"

static std::vector<Connection> connections;

void offending() {
  std::vector<Process> processes;
  for (Connection& conn : connections) {
    if (!conn)
      break;
    
    std::vector<Process> process = conn.getOS()->getProcesses();
    for (Process& p : process)
      processes.push_back(p);
  }
  std::sort(processes.begin(), processes.end(), [](Process a, Process b) -> bool {
    return a.time > b.time;
  });
  
  clear();
  refresh();
  
  int maxx, maxy;
  getmaxyx(stdscr, maxy, maxx);
  int action;
  do {
    clear();
    mvaddstr(0,0, "Host");
    mvaddstr(0,20, "Program");
    mvaddstr(0,40, "User");
    mvaddstr(0,50, "PID");
    mvaddstr(0,60, "CPU");
    mvaddstr(0,66, "MEM");
    mvaddstr(0,72, "Time");
    int row = 1;
    for (Process& p : processes) {
      mvaddnstr(row, 0, p.connection->getHostname().c_str(), 19);
      mvaddnstr(row, 20, p.command.c_str(), 19);
      mvaddnstr(row, 40, p.user.c_str(), 10);
      mvaddnstr(row, 50, std::to_string(p.pid).c_str(), 10);
      mvaddnstr(row, 60, std::to_string(p.cpu).c_str(), 5);
      mvaddnstr(row, 66, std::to_string(p.mem).c_str(), 5);
      mvaddnstr(row, 72, std::to_string(p.time / (60.0 * 60.0 * 24.0)).c_str(), 8);
      
      row++;
      if (row == maxy)
        break;
    }
    refresh();
    action = getchar();
    if (action == 'c') {
      std::sort(processes.begin(), processes.end(), [](Process a, Process b) -> bool {
        return a.cpu > b.cpu;
      });
    } else if (action == 'm') {
      std::sort(processes.begin(), processes.end(), [](Process a, Process b) -> bool {
        return a.mem > b.mem;
      });
    } else if (action == 't') {
      std::sort(processes.begin(), processes.end(), [](Process a, Process b) -> bool {
        return a.time > b.time;
      });
    }
    if (action == 'f') {
      while ((action = getch()) != KEY_BACKSPACE) {
        std::function<bool(Process)> func;
        switch (action) {
          case 'c':
            func = [](Process a) -> bool {
              return (a.cpu < 0.1);
            };
            break;
          case 'm':
            func = [](Process a) -> bool {
              return (a.mem < 0.1);
            };
            break;
          case 'u':
            func = [](Process a) -> bool {
              return (a.user == "root");
            };
            break;
          default:
            break;
        }
        
        if (func) {
          processes.erase(
              std::remove_if(processes.begin(), processes.end(), func),
              processes.end());
          break;
        }
      }
    
    }
  } while (action != 'q');
  
}

void filesystem() {
  std::vector<Filesystem> filesystems;
  
  for (Connection& conn : connections) {
    if (!conn)
      continue;
      
    std::vector<Filesystem> fss = conn.getOS()->getFilesystems();
    for (Filesystem& fs : fss) {
      filesystems.push_back(fs);
    }
  }
  clear();
  int action;
  int maxx, maxy;
  getmaxyx(stdscr, maxy, maxx);

  do {
    clear();
    mvaddstr(0,0,"Host");
    mvaddstr(0,20,"Source");
    mvaddstr(0,40,"Dest");
    mvaddstr(0,60,"Type");
    mvaddstr(0,65,"Usage");
    int row = 1;
    for (Filesystem& fs : filesystems) {
      mvaddnstr(row, 0, fs.connection->getHostname().c_str(), 19);
      mvaddnstr(row, 20, fs.source.c_str(), 19);
      mvaddnstr(row, 40, fs.target.c_str(), 19);
      mvaddnstr(row, 60, fs.type.c_str(), 4 );
      mvaddch(row, 65, '[');
      cchar_t t;
      t.attr = 0;
      t.chars[1] = L'\0';
      double usage = 12.0 * fs.used / fs.size;
      for (std::size_t i = 0; i < 12; i++) {
        int usg = (usage - i) * 9.0;
        usg = usg < 0 ? 0 : usg;
        usg = usg > 8 ? 8 : usg;
        t.chars[0] = L'\u2590' - usg;
        if (usg != 0) {
          add_wch( &t );
        } else {
          addch( ' ' );
        }
      }
      addch(']');
      row++;
      if (row > maxy)
        break;
    }
    action = getch();
    if (action == 'f') {
      std::sort(filesystems.begin(), filesystems.end(), [](Filesystem a, Filesystem b) -> bool {
        double frA = ((double)a.used) / a.size;
        double frB = ((double)b.used) / b.size;

        return (frA > frB);
      });
    }
  } while (action != 'q');
}

int main(int argc, char** argv) {
  Configuration conf;
  
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
  char action;
  int prog = 0;
  std::string progress = "|/-\\";
  start_color();
  init_pair(1, COLOR_RED, COLOR_BLACK);


  do {
    int row = 1;
    attron(A_DIM);
    mvaddstr(0, 0, "Hostname");
    mvaddstr(0,40, "| Load");
    mvaddstr(0,47, "| #usr");
    attroff(A_DIM);

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
        loadScaled = loadScaled > 7 ? 7 : loadScaled;
        loadScaled = loadScaled < 0 ? 0 : loadScaled;
        t.chars[0] = L'\u2581'+ loadScaled ;
        add_wch( &t );
      }
      
      mvaddstr(row, 47, "| ");
      addstr( std::to_string( conn.getOS()->getNUsers() ).c_str() );      
      
      row++;
    }
    refresh();
    row = 1;
    
    prog++;
    if (prog == 4)
      prog = 0;
    
    action = getch();
    if (action == 'o') {
      offending();
      clear();
    } else if (action == 'f') {
      filesystem();
      clear();
    }
  } while (action != 'q');
  
  endwin();
  
  return 0;
}
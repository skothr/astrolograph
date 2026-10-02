#ifndef MARKET_DATA_HPP
#define MARKET_DATA_HPP

#include <map>
#include <string>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <sstream>
#include <memory>

#include <curl/curl.h>

#include "dateTime.hpp"


struct MarketPoint
{
  double open   = 0.0;
  double close  = 0.0;
  double high   = 0.0;
  double low    = 0.0;
  double volume = 0.0;
  std::string date;
  std::string prevDate;
  std::string nextDate;
};
typedef std::map<std::string, MarketPoint> StockData;
typedef std::map<std::string, StockData> MarketData;



 


inline DateTime marketDate(const std::string &dateStr)
{
  std::stringstream ss(dateStr);
  std::string val;
  std::vector<std::string> vals;
  while(std::getline(ss, val, '-')) { vals.push_back(val); }

  if(vals.size() != 3)
    {
      std::cout << "ERROR: could not parse market date! --> (" << dateStr << ")\n";
      return DateTime();
    } 
  
  int year, month, day;
  ss.str(vals[0]); ss.clear();
  ss >> year;
  ss.str(vals[1]); ss.clear();
  ss >> month;
  ss.str(vals[2]); ss.clear();
  ss >> day;
  return DateTime(year, month, day, 0, 0, 0);
}

inline std::string marketDateStr(const DateTime &dt)
{
  std::stringstream ss;
  ss << std::setfill('0');
  ss << std::setw(4) << dt.year() << "-" << std::setw(2) << dt.month() << "-" << std::setw(2) << dt.day();
  return ss.str();
}



typedef std::pair<std::string, MarketPoint> Pair;
static bool compareDate(Pair i, Pair j) 
{ return i.first < j.first; }

// gets minimum date in data
inline DateTime minDate(const StockData &data) 
{
  if(data.size() == 0) { return DateTime(); }
  Pair minVal = *std::min_element(data.begin(), data.end(), &compareDate);
  return marketDate(minVal.first);
}

// gets minimum date in data
typedef std::pair<std::string, MarketPoint> Pair;
inline DateTime maxDate(const StockData &data) 
{
  if(data.size() == 0) { return DateTime(); }
  Pair maxVal = *std::max_element(data.begin(), data.end(), &compareDate);
  return marketDate(maxVal.first);
}



inline std::string csvFileName(const std::string &symbol)
{
  std::string sym = symbol;
  std::transform(sym.begin(), sym.end(), sym.begin(), [](unsigned char c) { return std::toupper(c); });
  return std::string("./data/") + sym + "-auto.csv";
}

// Example: "https://query1.finance.yahoo.com/v7/finance/download/SPY?period1=0&period2=3000000000&interval=1d&events=history&includeAdjustedClose=true"
#define HISTORICAL_DATA_PREFIX "https://query1.finance.yahoo.com/v7/finance/download/"

// callback for curl function
inline std::size_t curlCallbackMarket(const char* in, std::size_t size, std::size_t num, std::string *out)
{
  const std::size_t totalBytes(size*num);
  out->append(in, totalBytes);
  return totalBytes;
}

// returns path to downloaded data (./data/XXX-auto.csv)
inline std::string loadStockDataCurl(const std::string &symbol)
{
  std::string sym = symbol;
  std::transform(sym.begin(), sym.end(), sym.begin(), [](unsigned char c) { return std::toupper(c); });
  std::string savePath = csvFileName(sym);
  
  CURL *curl = curl_easy_init();
  if(curl)
    {
      std::string url = (std::string(HISTORICAL_DATA_PREFIX) + sym +
                         "?period1=-10000000000&period2=10000000000&interval=1d&events=history&includeAdjustedClose=true");

      std::cout << "Querying yahoo finance for " << sym << "...\n";
      std::cout << "  (URL: " << url << ")\n";
      
      std::unique_ptr<std::string> httpData(new std::string());
      
      curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
      curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curlCallbackMarket);
      curl_easy_setopt(curl, CURLOPT_WRITEDATA, httpData.get());
      std::cout << "...";
      CURLcode res = curl_easy_perform(curl);
      
      std::cout << "DONE\n";
      
      long httpCode = 0L;
      curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
      curl_easy_cleanup(curl);

      if(httpCode == 200)
        { // output to file
          std::cout << "WRITING TO CSV FILE: " << savePath << "\n";
          std::ofstream f(savePath);
          f << (*httpData.get());
        }
    }
  return savePath;
}

// loads historical data from a local CSV
inline StockData loadStockDataCsv(const std::string &csvPath)
{
  std::cout << "Loading market data file: " << csvPath << "\n";
  StockData data;
  
  std::ifstream f(csvPath, std::ios::in);
  std::string line = "";
  bool header = true;
  bool start = true;
  int colDate     = -1;
  int colOpen     = -1;
  int colHigh     = -1;
  int colLow      = -1;
  int colClose    = -1;
  int colAdjClose = -1; // TODO (?)
  int colVolume   = -1;
  int maxCol = -1; // maximum column index
  int row = 0;

  std::string prevDate;
  while(std::getline(f, line))
    {
      if(line.empty() || line == "\n") { std::cout << "Skipping empty line...\n"; continue; }
      std::string col;
      std::stringstream ss(line);
      std::vector<std::string> columns;
      while(std::getline(ss, col, ',')) // separate columns
        { std::transform(col.begin(), col.end(), col.begin(), [](unsigned char c) { return std::tolower(c); }); columns.push_back(col); }
      
      if(header)
        {
          for(int i = 0; i < columns.size(); i++)
            {
              bool found = false;
              const std::string &c = columns[i];
              if(c.find("date")       != std::string::npos)  { colDate = i; found = true; } // date
              else if(c.find("open")  != std::string::npos)  { colOpen = i; found = true; } // open
              else if(c.find("high")  != std::string::npos)  { colHigh = i; found = true; } // high
              else if(c.find("low")   != std::string::npos)  { colLow  = i; found = true; } // low
              else if(c.find("close") != std::string::npos)
                {
                  if(c.find("adj")    != std::string::npos)  { colAdjClose = i; found = true; } // close
                  else                                       { colClose    = i; found = true; } // adj close
                }
              else if(c.find("volume") != std::string::npos) { colVolume   = i; found = true; } // volume
              
              if(found) { maxCol = std::max(maxCol, i); }
            }
          
          if(colDate < 0 || colOpen < 0 || colHigh < 0 || colLow < 0 || colClose < 0 || colVolume < 0)
            {
              std::cout << "ERROR: Missing market data columns:\n";
              if(colDate   < 0) { std::cout << "  --> date\n";   }
              if(colOpen   < 0) { std::cout << "  --> open\n";   }
              if(colHigh   < 0) { std::cout << "  --> high\n";   }
              if(colLow    < 0) { std::cout << "  --> low\n";    }
              if(colClose  < 0) { std::cout << "  --> close\n";  }
              if(colVolume < 0) { std::cout << "  --> volume\n"; }
              std::cout << "\n";
              return { };
            }
          header = false;
        }
      else
        {
          if(columns.size() <= maxCol)
            {
              std::cout << "WARNING: Bad CSV row size! (row " << row << ")\n"
                        << "  -->  " << line << "\n Need " << maxCol << " columns.\n";
              continue;
            }

          std::string dateStr = columns[colDate];
          if(dateStr.empty() || dateStr.find("-") == std::string::npos) { std::cout << "====> Skipping blank date in row " << row << "!\n"; continue; }
          std::string dateVal = columns[colDate];
          std::replace(dateVal.begin(), dateVal.end(), '-', ' '); // replace dashes with spaces for stringstream
          std::replace(dateVal.begin(), dateVal.end(), '/', ' '); // replace slashes with spaces for stringstream
          std::stringstream ss(dateVal);
          int year;  ss >> year;
          int month; ss >> month;
          int day;   ss >> day;
          DateTime rowDt = DateTime(year, month, day, 0, 0, 0); rowDt.fix();
          
          data.emplace(dateStr, MarketPoint{});
          MarketPoint &d = data[dateStr];

          ss.clear(); ss.str(columns[colOpen]);   ss >> d.open;
          ss.clear(); ss.str(columns[colHigh]);   ss >> d.high;
          ss.clear(); ss.str(columns[colLow]);    ss >> d.low;
          ss.clear(); ss.str(columns[colClose]);  ss >> d.close;
          ss.clear(); ss.str(columns[colVolume]); ss >> d.volume;
          
          if(d.open == 0.0 && d.close != 0.0)      { d.open  = d.close; } // (some funky data)
          else if(d.close == 0.0 && d.open != 0.0) { d.close = d.open;  }

          d.date     = dateStr;
          d.prevDate = prevDate;
          if(!prevDate.empty()) { data[prevDate].nextDate = dateStr; }
          prevDate    = dateStr;
          
          // if(debug)
          //   {
          //     std::cout << dateVal << " (" << dt << " | index=" << d.dateIndex << ") --> \n"
          //               << "OPEN:   " << d.open   << "\n"
          //               << "CLOSE:  " << d.close  << "\n"
          //               << "HIGH:   " << d.high   << "\n"
          //               << "LOW:    " << d.low    << "\n"
          //               << "VOLUME: " << d.volume << "\n\n";
          //   }
          
          // if(d.low == 0 && d.close == 0)
          //   { // no data (weekend/holiday) -- copy previous day's close data
              
          //     MarketPoint &dLast = marketData[];
          //     d.open   = dLast.close;
          //     d.close  = dLast.close;
          //     d.high   = dLast.close;
          //     d.low    = dLast.close;
          //     d.volume = 0.0;
          //   }
        }
      row++;
    }

  
  auto iter = data.find("");
  if(iter != data.end())
    {
      std::cout << "====> Removing blank date in market data!\n";
      data.erase("");
    }

  for(auto &iter : data)
    {
      std::cout << "DATE: " << iter.first << " --> " << iter.second.close << "\n";
    }
  std::cout << "STOCK DATA SIZE: " << data.size() << " | CSV LINES: " << row << "\n";

  
  return data;
}


#endif // MARKET_DATA_HPP

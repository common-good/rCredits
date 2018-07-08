/**
 * @file
 * Display one or all Common Good data graphs
 * @param string accts
 * @param string funds
 * @param string velocity
 * @param string usd
 * @param string txs 
 * @param string topPct: a percent sign if Top 3 means top 3 percent, otherwise empty
 * A JSON-encoded data object is embedded in the page (see var ch below). Each element is a data object for one graph.
 * NOTE!: This script is used in an iframe of cg4.us/chart.php, which in turn includes this script (here for version control)
 */
var getv = parseUrlQuery($('#script-charts').attr('src').replace(/^[^\?]+\??/,''));
//alert($('#script-charts').attr('src').replace(/^[^\?]+\??/,''));
var ctty = getv['ctty'];
var chart = getv['chart'];
var site = getv['site'];

var ch = $('#chart-data').html();
ch = ch.substr(4, ch.length - 7); // trim off the comment markers
ch = JSON.parse(ch);
var vs = ch['vs'];
var dt1 = vs['dt1'];

$('#ctty').change(function () {recall(chart, $(this).val());});
$('#chart').change(function () {
  fixChartClass($(this));
  recall($(this).val(), ctty);}
);
fixChartClass($('#chart'));

var chartAreaW = '50%'; // leave room for yAxis labels and legend
var chartW = 480;
var chartH = 300;
if (getv['selectable']) {chartW = 600; chartH = 400;}

// (should be on member site instead) $('#edit-ctty').change(function () {window.location = baseUrl + '/community/graphs/qid=' + $(this).val();});

google.setOnLoadCallback(window[chart + 'Chart']);  

google.load('visualization', '1.0', {'packages':['corechart']});

function allChart() {
  growthChart();
  fundsChart();
  velocityChart();
  bankingChart();
  volumeChart();
//  issuedChart();
}

function dtFmt() {return (((new Date()).getTime() - dt1 * 1000) /1000/60/60/24/365.25 < 4) ? 'yyyy-MM' : 'yyyy';}

/**
 * Add a row to the table.
 * @param obj table: the gChart table object
 * @param string dataName: name of the dataset, embedded in the html
 * @param int remove: index of column to remove, if any
 */
function myRows(table, dataName, remove) {
  var dataSet = ch[dataName];
  for (i in dataSet) {
    dataSet[i][0] = new Date(dataSet[i][0] * 1000);
    if (remove) dataSet[i].splice(remove, 1); 
    table.addRow(dataSet[i]);    
  }
};

function growthChart() {
  var data = new google.visualization.DataTable();
  data.addColumn('date', 'Date');
  data.addColumn('number', 'Companies');
  data.addColumn('number', 'Members');
  data.addColumn('number', 'Joining');
  data.addColumn('number', 'Active');
//  data.addColumn('number', 'Conx');
//  data.addColumn('number', 'Local Conx');
  myRows(data, 'growthData');
  
  //ch['growthData']);

//seriesType:'bars',
//series: {5:{type:'line'}}
        
  var options = {
    title:'Accounts: ' + vs['accts'],
    width:chartW, height:chartH,
    series: [
      {areaOpacity:1, color:'blue'}, // bAccts
      {areaOpacity:1, color:'green'}, // pAccts
      {areaOpacity:0 , color:'silver'}, // newbs
      {areaOpacity:0, color:'red'} // aAccts
//      {areaOpacity:0, color:'yellow'}, // conx/aAcct
//      {areaOpacity:0, color:'orange'} // conxLocal/aAcct
    ],
    hAxis: {viewWindow: {min:new Date(dt1 * 1000)}, format:dtFmt(), gridlines: {count:5}, title:'', titleTextStyle: {color:'darkgray'}},
    chartArea: {width:chartAreaW},
    legend: {position:'right'}
  };

  var chart = new google.visualization.AreaChart(document.getElementById('growthChart'));
  chart.draw(data, options);
}

function fundsChart() {
  var data = new google.visualization.DataTable();
  data.addColumn('date', 'Date');
//  data.addColumn('number', 'Bals > 0');
  data.addColumn('number', 'CG Credits');
  data.addColumn('number', 'Dollar Pool');
  data.addColumn('number', 'Savings');
  data.addColumn('number', 'Top 3' + vs['topPct']);
  data.addColumn('number', 'Bottom 3' + vs['topPct']);
  data.addColumn('number', 'Credit Limits');
  data.addColumn('number', 'Bals < 0');
//  data.addRows(ch['fundsData']);
  myRows(data, 'fundsData');

  var options = {
    title:'Dollar Pool Total: ' + vs['funds'],
    width:chartW, height:chartH,
    series: [
//      {areaOpacity:0, color:'lime'}, // Bals > 0
      {areaOpacity:1, color:'#00cc00'}, // CG Credits (lighter green)
      {areaOpacity:1, color:'blue'}, // Dollar Pool
      {areaOpacity:0, color:'yellow'}, // Savings
      {areaOpacity:0, color:'red'}, // Top 3
      {areaOpacity:0, color:'red'}, // Bottom 3
      {areaOpacity:0, color:'magenta'},  // Limits
      {areaOpacity:1, color:'orange'} // Bals < 0
    ],
    hAxis: {format:dtFmt(), gridlines: {count:5}, title:'', titleTextStyle: {color:'darkgray'}},
    vAxis: {format:'short'},
    chartArea: {width:chartAreaW},
    legend: {position:'right'}
  };

  var chart = new google.visualization.AreaChart(document.getElementById('fundsChart'));
  chart.draw(data, options);
}

function velocityChart() {
  var data = new google.visualization.DataTable();
  data.addColumn('date', 'Date');
  data.addColumn('number', 'Inter-cmty');
  data.addColumn('number', 'Local');
  data.addColumn('number', 'Dollar Exchanges');
//  data.addRows(ch['velocityData']);
  myRows(data, 'velocityData');

  var options = {
    title:'Circulation Velocity: ' + vs['velocity'],
    width:chartW, height:chartH,
    series: [
      {areaOpacity:1, color:'yellow'}, // Inter-cmty
      {areaOpacity:1, color:'#00cc00'}, // Local
      {areaOpacity:0, color:'blue'} // USD Exchanges
    ],
    hAxis: {
      format:dtFmt(),
      gridlines: {count:5},
//      title:'What fraction of Common Good Credits turn over monthly', 
      titleTextStyle: {color:'darkgray'}
    },
    vAxis: {format:'percent', viewWindow:{min:0, max:1.5}},
    isStacked:false, // doesn't work
    chartArea: {width:chartAreaW},
    legend: {position:'right'}
  };

  var chart = new google.visualization.AreaChart(document.getElementById('velocityChart'));
  chart.draw(data, options);
}

function bankingChart() {
  var data = new google.visualization.DataTable();
  data.addColumn('date', 'Date');
  data.addColumn('number', 'FROM Bank');
  data.addColumn('number', 'TO Bank');
  if (ctty != 0) data.addColumn('number', 'Exports');
  data.addColumn('number', ctty == 0 ? 'Inter-cmty Trade' : 'Imports');

  myRows(data, 'bankingData', ctty == 0 ? 3 : 0);

  var options = {
    title:'Monthly USD Transfers: ' + vs['usd'],
    width:chartW, height:chartH,
    series: [
      {areaOpacity:1, color:'green'},
      {areaOpacity:1, color:'orange'},
      {areaOpacity:0, color:'lightblue'},
      {areaOpacity:0, color:'yellow'}
    ],
    hAxis: {format:dtFmt(), gridlines: {count:5}, title:'', titleTextStyle: {color:'darkgray'}},
    vAxis: {format:'short'},
    chartArea: {width:chartAreaW},
    legend: {position:'right'}
  };

  if (ctty == 0) options['series'].splice(3, 1); // remove "Exports", which actually means intra

  var chart = new google.visualization.AreaChart(document.getElementById('bankingChart'));
  chart.draw(data, options);
}

function volumeChart() {
  var data = new google.visualization.DataTable();
  data.addColumn('date', 'Date');
  data.addColumn('number', 'p2p');
  data.addColumn('number', 'p2b');
  data.addColumn('number', 'b2b');
  data.addColumn('number', 'b2p');
//  data.addRows(ch['volumeData']);
  myRows(data, 'volumeData');

  var options = {
    title:'Monthly Transactions: ' + vs['txs'],
    width:chartW, height:chartH,
    colors: ['orange', 'green', 'blue', 'red'],
    hAxis: {format:dtFmt(), gridlines: {count:5}},
//    hAxis: {format:dtFmt(), gridlines: {count:5}, title:'(logarithmic scale)', titleTextStyle: {color:'darkgray'}},
//    vAxis: {logScale:true},
    chartArea: {width:chartAreaW},
    legend: {position:'right'}
  };

  var chart = new google.visualization.LineChart(document.getElementById('volumeChart'));
  chart.draw(data, options);
}

function recall(chart, ctty) {
  var myUrl = site == 'dev' ? 'http://localhost/cgMembers/rcredits/misc' : 'https://cg4.us';
  window.location = myUrl + '/chart.php?selectable=1&chart=' + chart + '&ctty=' + ctty + '&site=' + site;
};

function fixChartClass(context) {
  $('option', context).removeClass();
  $(':selected', context).addClass('selected');
}
var readme = $.get("README.md", function(data) {
  var lines = data.split("\n");
  console.log(lines[1]);
  $('<div id="readme-title" class="page-header"><h1>'+lines[1]+'</h1></div>').appendTo("#readme");
  lines.splice(0, 3);
  $('<div id="readme-body" class="caption"></div>').appendTo("#readme");
  for( var i=0; i < lines.length; i++){
    $('<div><p class="lead">'+lines[i]+'</p></dvi>').appendTo("#readme-body");
  }
});

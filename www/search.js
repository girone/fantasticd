console.log("Javascript loaded.")

$(document).ready(function() {
  $("#input").keypress(function(e) {
    var query = $("#input").val()
    if (e.which == 13) {
      search(query)
    } else {

      var host = window.location.host
      //var port = window.location.port
      var url = "http://" + host + "/?q=" + query
      //$.get(url, function(result) {
      //  console.log(result)
      //})
    }
  })

  // TODO(Jonas): Suggest autocompletion only for the current word, keep previously typed ones.
  // TODO(Jonas): Better layout for the suggestions.
  $("#input").autocomplete({
    source : function(request, response) {
      // Determine the current typed word, save already completed.
      var term = request.term
      var offset_of_last_space = term.lastIndexOf(" ")
      var finished = term.substring(0, offset_of_last_space + 1)
      var input = term.substring(offset_of_last_space + 1)
      if (input.length < 3) { return }

      // Request autocompletion from the server.
      var host = window.location.host
      var url = "http://" + host + "/?ac=" + input
      console.log("Requesting autocompletion from \"" + url + "\"")
      // TODO(Jonas): Use AJAX instead of blocking $.get ($.ajaxJSON()).
      $.get(url, function(data) {
        console.log("Received " + data)
        // Distinguish already typed and suggestion by highlight.
        //var tmp = []
        //$.each(data, function(index, value) {
        //  value = "<strong>" + finished + "</strong>" + value
        //  console.log(value)
        //  tmp.push(value)
        //})
        //data = tmp

        response($.map(data, function(item) {
          return {label: "<strong>" + finished + "</strong>" + item,
                  value: finished + item}
        }))
      }, 'json')
    },
    minLength: 3
  })
  .data('ui-autocomplete')._renderItem = function( ul, item ) {
    console.log("Custom renderer called.");
    return $( "<li></li>" )
      .data( "ui-autocomplete-item", item )
      .append( '<a>' + item.label + '</a>' )
      .appendTo( ul );
  };
})

function search(query) {
  var url = "http://" + window.location.host + "/q="
  $.get(url, function(data) {
    console.log("Query " + url + " got response from server: " + data)
  });
}


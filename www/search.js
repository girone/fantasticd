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
      // TODO(Jonas): Use AJAX instead of blocking $.get ($.ajaxJSON()), if that is different.
      $.get(url, function(data) {
        console.log("Received " + data)
        // Distinguish already typed and suggestion by highlight.
        response($.map(data, function(item) {
          return {label: "<strong>" + finished + "</strong>" + item,
                  value: finished + item}
        }))
      }, 'json')
    },
    minLength: 3
  })
  .data('ui-autocomplete')._renderItem = function( ul, item ) {
    return $( "<li></li>" )
      .data( "ui-autocomplete-item", item )
      .append( '<a>' + item.label + '</a>' )
      .appendTo( ul );
  };
})

function search(query) {
  var url = "http://" + window.location.host + "/?q=" + query
  var result = $.getJSON(url, function(data) {
    console.log("Query " + url + " got response from server: ")
    console.log(data)
    
    data = $.map(data, function(entry) {
      var li = "<li class=\"result_li\">" +
               "<a href=\"" + entry.url + "\" target=\"_blank\">" + 
               "<span class=\"li_title\">" + entry.title + "</span><br>" + 
               "<span class=\"li_content\">" + entry.content + "</span>" +
               "</a></li>";
      return li;
    }); 
    var html = "";
    $.each(data, function(index, value) {
      html += value + "\n"
    });
    show_result(html);
  });
  //result.fail(function(data, text_status, error) {
  //  show_result("<strong>Error while querying the search server.</strong>")
  //});
}

function show_result(text) {
    $("#search_list").html(text);
    $("#search_results").show();
}


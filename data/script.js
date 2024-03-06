const gridItem = document.getElementById("grid-item");
const enrollForm = document.getElementById("enroll-form");
const enrollTextField = document.getElementById("enroll-text-field");
const unlockButton = document.getElementById("unlock-button");
const deleteAllButton = document.getElementById("delete-all-button");

deleteAllButton.addEventListener("click", () => {
  console.log(`Button to delete all users pressed!`);
  if (confirm(`Are you sure you want to delete all user fingerprints?`)) {
    fetch(`/deleteall`, {
      method: 'DELETE',
    })
      .then(response => {
        if (!response.ok) { throw new Error('Network response was not ok'); }
        console.log('User deleted successfully');
        getData(); // Refresh the front end user display (hopefully!!)
      })
      .catch(error => { console.error('There was a problem with the fetch operation:', error); });

  }
   

});

unlockButton.addEventListener("click", () => {
  fetch("/unlock", {method: 'GET'})
    .then(() => {
      // Optional: Handle success if needed
      console.log("GET request sent successfully");
    })
    .catch(error => {
      // Handle errors
      console.error("Error sending GET request:", error);
    });
});

enrollForm.addEventListener("submit", function(event) {
  event.preventDefault();
  if (enrollTextField.validity.valid) {
    fetch(`/enroll/users/?param1=${enrollTextField.value}`, { method: 'POST' })
      .then(response => {
        if (!response.ok) {
          throw new Error('Network response was not ok');
        }
        enrollTextField.value = "";
        // alert("Finish Enrollment Process on Sensor");
        getData();
      })
      .catch(error => { console.error('There was a problem with the fetch operation:', error); });
  } else {
    event.preventDefault();
  }

});

document.addEventListener('DOMContentLoaded', () => {
  console.log("fetching!");
  getData();
});

setInterval(getData, 2500);

function getData() {
  fetch('/database_info', { method: 'GET' })
    .then(response => {
      if (!response.ok) { throw new Error('Network response was not ok'); }
      return response.json(); // Parse response as JSON
    })
    .then(renderUsers) // Handle response
    .catch(error => { console.log("error"); });
}


function renderUsers(data) {
  userGridClass = Array.from(document.getElementsByClassName("grid-item-class")).slice(1);
  userGridClass.forEach(userGrid => {
      userGrid.remove();
  })

  data.forEach(user => {
    var gridItemClone = gridItem.cloneNode(true);
    gridItemClone.classList.add("grid-item-class");
    gridItemClone.style.display = "flex";
    const individualUser = gridItemClone.querySelector('p');
    individualUser.classList.add("user-name");
    individualUser.innerHTML = user;
    const deleteButton = gridItemClone.querySelector('button');
    deleteButton.classList.add("delete-button-class");
    deleteButton.id = user;
    document.body.appendChild(gridItemClone);

    deleteButton.addEventListener("click", () => deleteUser(deleteButton.id));
  })
}

function deleteUser(nameToDelete) {
  console.log(`Button to delete ${nameToDelete} pressed!`);
  if (confirm(`Are you sure you want to delete ${nameToDelete}?`)) {
    fetch(`/delete/users/?param1=${nameToDelete}`, {
      method: 'DELETE',
    })
      .then(response => {
        if (!response.ok) { throw new Error('Network response was not ok');}
        console.log('User deleted successfully');
        getData(); // Refresh the front end user display (hopefully!!)
      })
      .catch(error => { console.error('There was a problem with the fetch operation:', error); });
  } else {
    console.log("aborted");
  }
}
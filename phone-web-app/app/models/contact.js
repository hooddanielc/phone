import DS from 'ember-data';

const model = DS.Model.extend({
  firstName: "John",
  lastName: "Doe",
  number: "+12061231234"
});

let _tmpID = 0;
const newID = function () {
  return ++_tmpID;
};

model.reopenClass({
  FIXTURES: [
    { id: newID(), firstName: 'John', lastName: 'Doe', number: "+12061231234" },
    { id: newID(), firstName: 'Bill', lastName: 'Gates', number: "+12061231234" },
    { id: newID(), firstName: 'Tom', lastName: 'Hanks', number: "+12061231234" },
    { id: newID(), firstName: 'Robin', lastName: 'Hood', number: "+12061231234" },
    { id: newID(), firstName: 'Richard', lastName: 'Mussle', number: "+12061231234" },
    { id: newID(), firstName: 'Destry', lastName: 'White', number: "+12061231234" },
    { id: newID(), firstName: 'Amy', lastName: 'Winehouse', number: "+12061231234" },
    { id: newID(), firstName: 'Amylia', lastName: 'Lynch', number: "+12061231234" },
    { id: newID(), firstName: 'John', lastName: 'Doe', number: "+12061231234" },
    { id: newID(), firstName: 'Bill', lastName: 'Gates', number: "+12061231234" },
    { id: newID(), firstName: 'Tom', lastName: 'Hanks', number: "+12061231234" },
    { id: newID(), firstName: 'Robin', lastName: 'Hood', number: "+12061231234" },
    { id: newID(), firstName: 'Richard', lastName: 'Mussle', number: "+12061231234" },
    { id: newID(), firstName: 'Destry', lastName: 'White', number: "+12061231234" },
    { id: newID(), firstName: 'Amy', lastName: 'Winehouse', number: "+12061231234" },
    { id: newID(), firstName: 'Amylia', lastName: 'Lynch', number: "+12061231234" },
    { id: newID(), firstName: 'John', lastName: 'Doe', number: "+12061231234" },
    { id: newID(), firstName: 'Bill', lastName: 'Gates', number: "+12061231234" },
    { id: newID(), firstName: 'Tom', lastName: 'Hanks', number: "+12061231234" },
    { id: newID(), firstName: 'Robin', lastName: 'Hood', number: "+12061231234" },
    { id: newID(), firstName: 'Richard', lastName: 'Mussle', number: "+12061231234" },
    { id: newID(), firstName: 'Destry', lastName: 'White', number: "+12061231234" },
    { id: newID(), firstName: 'Amy', lastName: 'Winehouse', number: "+12061231234" },
    { id: newID(), firstName: 'Amylia', lastName: 'Lynch', number: "+12061231234" },
    { id: newID(), firstName: 'John', lastName: 'Doe', number: "+12061231234" },
    { id: newID(), firstName: 'Bill', lastName: 'Gates', number: "+12061231234" },
    { id: newID(), firstName: 'Tom', lastName: 'Hanks', number: "+12061231234" },
    { id: newID(), firstName: 'Robin', lastName: 'Hood', number: "+12061231234" },
    { id: newID(), firstName: 'Richard', lastName: 'Mussle', number: "+12061231234" },
    { id: newID(), firstName: 'Destry', lastName: 'White', number: "+12061231234" },
    { id: newID(), firstName: 'Amy', lastName: 'Winehouse', number: "+12061231234" },
    { id: newID(), firstName: 'Amylia', lastName: 'Lynch', number: "+12061231234" },
    { id: newID(), firstName: 'John', lastName: 'Doe', number: "+12061231234" },
    { id: newID(), firstName: 'Bill', lastName: 'Gates', number: "+12061231234" },
    { id: newID(), firstName: 'Tom', lastName: 'Hanks', number: "+12061231234" },
    { id: newID(), firstName: 'Robin', lastName: 'Hood', number: "+12061231234" },
    { id: newID(), firstName: 'Richard', lastName: 'Mussle', number: "+12061231234" },
    { id: newID(), firstName: 'Destry', lastName: 'White', number: "+12061231234" },
    { id: newID(), firstName: 'Amy', lastName: 'Winehouse', number: "+12061231234" },
    { id: newID(), firstName: 'Amylia', lastName: 'Lynch', number: "+12061231234" }
  ]
});

export default model;
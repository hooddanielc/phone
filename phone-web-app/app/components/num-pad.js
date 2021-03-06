import Ember from 'ember';

export default Ember.Component.extend({
  classNames: ['num-pad'],
  input: '',

  focusInput: function () {
    window.theThis = this;
    this.$('input').focus();
  }.on('didInsertElement'),

  actions: {
    showMenuPress: function () {
      this.$('input').focus();
    },

    backspaceButtonPress: function () {
      this.set('input', this.get('input').substring(0, this.get('input.length') - 1));
      this.$('input').focus();
    },

    numpadButtonPress: function (num) {
      this.set('input', this.get('input') + num);
      this.$('input').focus();
    },

    call: function () {
      console.log('call');
    },

    email: function () {
      console.log('email');
    },

    add: function () {
      console.log('add');
    }
  }
});

import Ember from 'ember';

export default Ember.Component.extend({
  input: '',

  focusInput: function () {
    window.theThis = this;
    this.$('input').focus();
  }.on('didInsertElement'),

  actions: {
    showMenuPress: function () {
      console.log('showMenuPress');
      this.$('input').focus();
    },

    backspaceButtonPress: function () {
      this.set('input', this.get('input').substring(0, this.get('input.length') - 1));
      this.$('input').focus();
    },

    numpadButtonPress: function (num) {
      this.set('input', this.get('input') + num);
      this.$('input').focus();
    }
  }
});

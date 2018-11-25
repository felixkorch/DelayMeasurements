<script>
  import { Line } from 'vue-chartjs'
 
  export default {
    name: 'line-chart',
    extends: Line,
    props: ['data', 'options'],
    mounted () {
      this.renderChart(this.data, this.options)
    }
  }
</script>
<template>
    <div class="measurements_table">
        <dl class="dl-horizontal">
            <dt>Average</dt>
            <dd> {{ measurements.average }} ms </dd>
            <dt>Max. response time</dt>
            <dd> {{ measurements.maxDuration }} ms </dd>
            <dt>Min. response time</dt>
            <dd> {{ measurements.minDuration }} ms </dd>
            <dt>Max. deviation</dt>
            <dd> {{ measurements.maxDeviation }} ms </dd>
            <dt>Interval</dt>
            <dd> {{ measurements.interval / 1000 }} s </dd>
            <dt>Data points</dt>
            <dd> {{ measurements.length }} </dd>
        </dl>
    </div>
</template>

<script>
    export default {
        name: 'measurements-table',
        props: {
            passedData: Object
        },
        data() {
            return {
            }
        },

        methods: {           
        },

        computed: {
            measurements: function() {
                if (this.passedData)
                    return this.passedData;
                else
                    return {};
            }
        }
    }
</script>

<style scoped>
.measurements_table {
    
}

.dl-horizontal {
    font-size: 18px;
}
dd {
    font-weight: bold;
    background-color:  #eaf0fa;
    padding: 7px;
}

ul {
    /* list-style-type: none; */
    margin: 10px;
    padding: 10px;
}

.measurements-responsive {
    max-height: 320px;
}

</style>
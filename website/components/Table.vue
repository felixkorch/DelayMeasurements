<template>
    <div class="measurements_table">
        <dl class="dl-horizontal">
            <dt>Average</dt>
            <dd> {{ measurements.average }} ms </dd>
            <dt>Max. response time</dt>
            <dd> {{ measurements.max_duration }} ms </dd>
            <dt>Min. response time</dt>
            <dd> {{ measurements.min_duration }} ms </dd>
            <dt>Max. deviation</dt>
            <dd> {{ measurements.max_deviation }} ms </dd>
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